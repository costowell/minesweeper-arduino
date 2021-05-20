#include <Adafruit_GFX.h>
#include <Adafruit_TFTLCD.h>
#include <TouchScreen.h>

// -- Pin -- //

#define YP A3
#define XM A2
#define YM 9
#define XP 8

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

// -- Declare Colors -- //

/// General Colors

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

/// Specific Colors

#define GREEN_LIGHT 0xAAD751
#define GREEN_DARK 0xAEAA
#define GREEN_DARKER 0x4ba5
#define TAN_LIGHT 0xe613
#define TAN_DARK 0xd5b3
#define RED_LIGHT 0xFBCE

// -- Compilation Control -- //

#define SERIAL_ENABLED 1

// -- Declare Draw Board Values -- //

#define BOARDW 6
#define BOARDH 6
#define BOXSIZE 40
#define TILECOUNT BOARDW*BOARDH
#define MINECOUNT 5

// -- Declare Text Values -- //

#define TEXT_SIZE 3
#define TEXT_X_OFFSET 13
#define TEXT_Y_OFFSET 10

// -- Declare Board Values -- //

#define MINE -1
#define UNDISCOVERED -2
#define UN -2 // For when I'm initializing a 6x6 array
#define FLAG -3

// -- Declare Hardware Values -- //

#define MINPRESSURE 10
#define MAXPRESSURE 1000
#define BAUDRATE 9600

// -- Global Vars -- //

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

int pubboard[6][6] = {
                      {UN, UN, UN, UN, UN, UN},
                      {UN, UN, UN, UN, UN, UN},
                      {UN, UN, UN, UN, UN, UN},
                      {UN, UN, UN, UN, UN, UN},
                      {UN, UN, UN, UN, UN, UN},
                      {UN, UN, UN, UN, UN, UN},
                    };
int prvboard[6][6] = {
                      0, 0, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0,  
                      0, 0, 0, 0, 0, 0,  
                      0, 0, 0, 0, 0, 0,  
                      0, 0, 0, 0, 0, 0,  
                      0, 0, 0, 0, 0, 0,                      
                    };
int cursor[2] = {-1, -1};

int currentTileCount = TILECOUNT;
unsigned long startTime;
int prevColor = WHITE;
bool isGameOver = false;
bool isInitialized = false;

// -- Code -- //

void drawFlag(int x, int y) {
  tft.fillRoundRect(x +  4, y + BOXSIZE - 8, 32, 4 , 5, RED_LIGHT);
  tft.fillRoundRect(x + 18, y + 6, 4 , 30, 5, RED_LIGHT);
  tft.fillTriangle(x + 18, y + 6, 
                   x + 18, y + 22,
                   x + 4 , y + 14, 
                   RED_LIGHT);
}

void drawBoard() {
  for(int y = 0; y < 6; y++) {
    for (int x = 0; x < 6; x++) {
      if (pubboard[y][x] != UNDISCOVERED && pubboard[y][x] != FLAG) tft.fillRect(BOXSIZE * x, BOXSIZE * y, BOXSIZE, BOXSIZE, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT);
      else tft.fillRect(BOXSIZE * x, BOXSIZE * y, BOXSIZE, BOXSIZE, ((x + y) & 1) == 1 ? GREEN_DARK : GREEN_LIGHT);
      switch (pubboard[y][x]) {          
        case 1:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '1', BLUE, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;
        case 2:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '2', RED, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;
        case 3:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '3', CYAN, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;
        case 4:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '4', MAGENTA, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;
        case 5:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '5', GREEN, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;
        case 6:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '6', BLACK, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;
        case 7:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '7', BLUE, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;
        case 8:
          tft.drawChar((BOXSIZE * x) + TEXT_X_OFFSET, (BOXSIZE * y) + TEXT_Y_OFFSET, '8', GREEN, ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT, TEXT_SIZE);
          break;          
        case FLAG:
          drawFlag(BOXSIZE * x, BOXSIZE * y);
          break;
      }
    }
  }
}

void printCentered(const char * str, int strsize, int offset, int height, int fontsize, int color) {
  tft.setTextSize(fontsize);
  tft.setTextColor(color);
  tft.setCursor(((tft.width() - (strsize * fontsize * 5)) / 2) - offset, height);
  tft.println(str);
}

void printCenteredShadow(const char * str, int strsize, int height, int fontsize, int color) {
  printCentered(str, strsize, 2, height + 2, fontsize, BLACK);
  printCentered(str, strsize, 0, height, fontsize, color);
}

bool initiateGameOver(bool won) {
  isGameOver = true;
  if (!won) {
    tft.fillCircle(BOXSIZE * cursor[0] + 20, BOXSIZE * cursor[1] + 20, 10, ((cursor[0] + cursor[1]) & 1) == 0 ? GREEN_DARK : GREEN_LIGHT);
    printCenteredShadow("Game over!", sizeof("Game over!"), BOXSIZE * 2, TEXT_SIZE, RED_LIGHT);
    delay(1000);
    printCenteredShadow("Continue?", sizeof("Continue?"), BOXSIZE * 3, TEXT_SIZE - 1, RED-10);
    return false;
  } else {
    String secs = String((millis() - startTime) / 1000);
    
    Serial.println(secs.concat(" secs"));
    printCenteredShadow("You win!", sizeof("You win!"), BOXSIZE*2 + 0, TEXT_SIZE, GREEN);
    delay(1000);
    printCenteredShadow(secs.c_str(), secs.length() + 1, BOXSIZE*3, TEXT_SIZE, WHITE);
    delay(1000);
    printCenteredShadow("Continue?", sizeof("Continue?"), BOXSIZE * 4, TEXT_SIZE - 1, RED-10);
    return true;
  }
}

void initBoard() {
  // Gen bombs
  int mineCount = MINECOUNT;

  // Loop until MINECOUNT amount of mines are on the board
  while(mineCount != 0) {
    int randX = random(6);
    int randY = random(6);
    
    // Make sure none of the mines appear around the cursor's point
    if (prvboard[randY][randX] == 0 && (randX != cursor[0]     || randY != cursor[1]     ) 
                                    && (randX != cursor[0] + 1 || randY != cursor[1]     )
                                    && (randX != cursor[0] + 1 || randY != cursor[1] - 1 )
                                    && (randX != cursor[0] + 1 || randY != cursor[1] + 1 )
                                    && (randX != cursor[0]     || randY != cursor[1] + 1 )
                                    && (randX != cursor[0]     || randY != cursor[1] - 1 )
                                    && (randX != cursor[0] - 1 || randY != cursor[1]     )
                                    && (randX != cursor[0] - 1 || randY != cursor[1] + 1 )
                                    && (randX != cursor[0] - 1 || randY != cursor[1] - 1 )) {
      // Set the mine and move onto next iteration
      prvboard[randY][randX] = MINE;
      mineCount--;
    }
  }
  
  // Fill in non-bomb spaces with numbers
  for(int y = 0; y < 6; y++) {
    for(int x = 0; x < 6; x++) {
      // If a mine is found, increment the surrounding tiles by one
      if (prvboard[y][x] == MINE) {
        if(y != 0 && x != 0 &&  prvboard[y - 1][x - 1] != MINE)            prvboard[y - 1][x - 1]++;
        if(y != 0 &&            prvboard[y - 1][x    ] != MINE)            prvboard[y - 1][x    ]++;
        if(y != 0 && x != 5 &&  prvboard[y - 1][x + 1] != MINE)            prvboard[y - 1][x + 1]++;
        if(x != 0 &&            prvboard[y    ][x - 1] != MINE)            prvboard[y    ][x - 1]++;
        if(x != 5 &&            prvboard[y    ][x + 1] != MINE)            prvboard[y    ][x + 1]++;
        if(y != 5 && x != 0 &&  prvboard[y + 1][x - 1] != MINE)            prvboard[y + 1][x - 1]++;
        if(y != 5 &&            prvboard[y + 1][x    ] != MINE)            prvboard[y + 1][x    ]++;
        if(y != 5 && x != 5 &&  prvboard[y + 1][x + 1] != MINE)            prvboard[y + 1][x + 1]++;
      }
    }
  }
  // Start the timer
  Serial.println(startTime);
  startTime = millis();
  Serial.println(startTime);
  isInitialized = true;
}

// Returns false if a mine is dug
bool dig(int x, int y) {
  // Make sure that digging isn't done at the default {-1, -1} value.
  // Also prevent player from digging flags and already discovered tiles
  if (x < 0 || x > 5 || y < 0 || y > 5 || pubboard[y][x] != UNDISCOVERED || pubboard[y][x] == FLAG) return true;
  if(!isInitialized) initBoard();
  // If mine is dug
  if(prvboard[y][x] == -1) return initiateGameOver(false);

  
  // "Dig" the selected tile and decrement the total tile count
  pubboard[y][x] = prvboard[y][x];
  --currentTileCount;

  // If there is a 0, we should recurse until there are no more 0s adjacent to one another
  if (pubboard[y][x] != 0) return true;
  dig(x - 1, y - 1); 
  dig(x    , y - 1); 
  dig(x + 1, y - 1); 
  dig(x - 1, y    ); 
  dig(x + 1, y    ); 
  dig(x - 1, y + 1); 
  dig(x    , y + 1); 
  dig(x - 1, y + 1);

  return true;
}

bool flag(int x, int y) {
  // Make sure that digging isn't done at the default {-1, -1} value.
  // Prevent placing on discovered or unflaged tiles
  if (x < 0 || x > 5 || y < 0 || y > 5 || (pubboard[y][x] != UNDISCOVERED && pubboard[y][x] != FLAG)) return false;

  // Toggle tile's flagged state
  if (pubboard[y][x] == FLAG) {
    pubboard[y][x] = UNDISCOVERED;
    tft.fillRect(BOXSIZE * x, BOXSIZE * y, BOXSIZE, BOXSIZE, ((x + y) & 1) == 1 ? GREEN_DARK : GREEN_LIGHT);
  } else {
    pubboard[y][x] = FLAG;
  }
  
  return true;
}

void reinit() {
  
  for(int y = 0; y < 6; y++) {
    for(int x = 0; x < 6; x++) {
      pubboard[y][x] = UNDISCOVERED;
      prvboard[y][x] = 0;
    }
  }

  cursor[0] = -1;
  cursor[1] = -1;

  currentTileCount = TILECOUNT;
  prevColor = WHITE;
  isGameOver = false;
  isInitialized = false;

  drawBoard();
}

void setup() {
  // Begin Serial if SERIAL_ENABLED is 1
  #if (SERIAL_ENABLED == 1)
    Serial.begin(BAUDRATE);
    Serial.println(F("Serial enabled! Setting up..."));
  #endif

  tft.reset();

  // Get hardware ID
  uint16_t identifier = tft.readID();

  // Print hardware ID
  #if (SERIAL_ENABLED == 1)
    if(identifier == 0x9325) {
      Serial.println(F("Found ILI9325 LCD driver"));
    } else if(identifier == 0x9328) {
      Serial.println(F("Found ILI9328 LCD driver"));
    } else if(identifier == 0x7575) {
      Serial.println(F("Found HX8347G LCD driver"));
    } else if(identifier == 0x9341) {
      Serial.println(F("Found ILI9341 LCD driver"));
    } else if(identifier == 0x8357) {
      Serial.println(F("Found HX8357D LCD driver"));
    } else {
      Serial.print(F("Unknown LCD driver chip: "));
      Serial.println(identifier, HEX);
      Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
      Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
      Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
      Serial.println(F("If using the breakout board, it should NOT be #defined!"));
      Serial.println(F("Also if using the breakout, double-check that all wiring"));
      Serial.println(F("matches the tutorial."));
      return;
    }
  #endif

  // Set seed from noise from pin 5
  randomSeed(analogRead(5));

  // Initialize LCD
  tft.begin(identifier);

  // Draw the empty board
  drawBoard();

  // Draw the bottom components

  /// Fill bottom
  tft.fillRect(0, BOXSIZE*6, TS_MAXX, BOXSIZE*2, GREEN_DARKER);

  /// Draw buttons
  tft.fillRoundRect(BOXSIZE*1, BOXSIZE*6.5, BOXSIZE*1.2, BOXSIZE*1.2, 10, GREEN_DARK);
  tft.fillRoundRect(BOXSIZE*3.8, BOXSIZE*6.5, BOXSIZE*1.2, BOXSIZE*1.2, 10, GREEN_DARK);

  /// Draw icons on buttons

  tft.fillRect(BOXSIZE + 8, BOXSIZE*6.5 + 8, 17, 17, TAN_DARK);
  tft.fillRect(BOXSIZE + 23, BOXSIZE*6.5 + 8, 17, 17, TAN_LIGHT);
  tft.fillRect(BOXSIZE + 8, BOXSIZE*6.5 + 23, 17, 17, TAN_LIGHT);
  tft.fillRect(BOXSIZE + 23, BOXSIZE*6.5 + 23, 17, 17, TAN_DARK);

  drawFlag(BOXSIZE * 3.9, BOXSIZE * 6.6);

}

void loop() {
  // put your main code here, to run repeatedly:
  TSPoint p = ts.getPoint();

  // I don't know what this does, but for whatever reason it breaks if I remove it
  // "Legacy Code"
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  // If the preassure is within the bounds
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {    
    // Maps the uneven touch input to the width and height of the screen
    p.x = tft.width() - map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);    

    // Restart the game
    if (isGameOver) return reinit();

    // If the click is within the bounds of the board
    if (p.y < BOXSIZE * 6) {
      // Loop over all of the cells
      for(int y = 0; y < 6; y++) {
        for(int x = 0; x < 6; x++) {
          // Once the selected cell is found, check if the selection changed         
          if (p.x > x * BOXSIZE && p.x < (x * BOXSIZE) + BOXSIZE
            && (p.y > y * BOXSIZE && p.y < (y * BOXSIZE) + BOXSIZE)
            && (cursor[0] != x || cursor[1] != y)) {
              // Replace the white selection box with the appropriate color box around previous selection
              tft.drawRect(cursor[0] * BOXSIZE, cursor[1] * BOXSIZE, BOXSIZE, BOXSIZE, prevColor);

              // Get new selection and draw a white border around it              
              cursor[0] = x;
              cursor[1] = y;
              if (pubboard[y][x] != UNDISCOVERED) prevColor = ((x + y) & 1) == 1 ? TAN_DARK : TAN_LIGHT;
              else prevColor = ((x + y) & 1) == 1 ? GREEN_DARK : GREEN_LIGHT;
              tft.drawRect(x * BOXSIZE, y * BOXSIZE, BOXSIZE, BOXSIZE, WHITE);
          }
        }        
      }
    
    } else if (p.x > BOXSIZE && p.x < BOXSIZE * 2.2) { // Dig Button
      if (dig(cursor[0], cursor[1])) drawBoard();
      if (currentTileCount == MINECOUNT) initiateGameOver(true);
    } else if (p.x > BOXSIZE * 3.8 && p.x < BOXSIZE * 5) { // Flag Button
      if (flag(cursor[0], cursor[1])) drawBoard();
    }
  } 
}
