#include "LedControl.h"
#include "Menu.h"
#include "LiquidCrystal.h"
#include "JoystickManager.h"
#include "EEPROMManager.h"

// Joystick
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickSwitchPin = 2;
const bool joystickCommonAnode = false;

JoystickManager joystickManager(joystickXPin, joystickYPin, joystickSwitchPin, joystickCommonAnode);
EEPROMManager eepromManager;

// Matrix
const byte dinPin = 12;
const byte clockPin = 11;
const byte loadPin = 10;
const byte matrixSize = 8;
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1);
byte matrixBrightness = 2;

// Menu
int difficulty = 1;  // TODO: get from lcd menu for project
int maxDifficulty = 15;
int lastImgIdx = 0;
unsigned long lastLoadingImageDisplayTime = 0;
const int loadingImageChangeTime = 500;
bool playerMoved = false;

// Game
const int boardSize = 8;
int board[boardSize][boardSize];
bool shouldResetGame = true;
int playerPositionRow;
int playerPositionCol;
const int empty = 0;
const int wall = 1;
const int bomb = 2;
unsigned long lastPlayerBlinkTime;
const int slowBlinkTime = 500;
unsigned long lastBombBlinkTime = 0;
const int fastBlinkTime = 100;
unsigned long bombPlacementTime = 0;
bool bombPlaced = false;
const int bombExplosionTime = 1600;
unsigned long explodeAnimationTime = 400;
const int startDelayForBombPlacement = 500;
int bombPositionRow = -1;
int bombPositionCol = -1;
int bombsUsed = 0;
unsigned long lastSavedElapsedTime = 0;
unsigned long elapsedTime;  // in seconds
const int oneSecondInMs = 1000;
bool waitingForUserInputEndGame = true;
enum GameState {
  IN_MENU,
  PLAYING,
  LOST,
  WON,
  RESET
};
GameState gameState = IN_MENU;

// Animations
const uint64_t loadingStateImages[] = {
  0x0000000000000000,
  0x0000040000000000,
  0x00000c0000000000,
  0x00001c0000000000,
  0x00003c0000000000
};
const int imagesLen = sizeof(loadingStateImages) / 8;
byte smile[8] = { 0x3C, 0x42, 0xA5, 0x81, 0xA5, 0x99, 0x42, 0x3C };
byte sad[8] = { 0x3C, 0x42, 0xA5, 0x81, 0x99, 0xA5, 0x42, 0x3C };

// LCD
const byte rs = 13;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const int lcdContrastPin = 9;
const int lcdBrightnessPin = 3;

// LCD menu stuff
Menu lcdMenu(lcd, joystickManager, eepromManager);
byte bombChar[] = {
  B00010,
  B00100,
  B01110,
  B10001,
  B10001,
  B10001,
  B01110,
  B00000
};
const char* endGameMessages[2] = { "Good job!", "Keep trying!" };

void setup() {
  Serial.begin(9600);

  pinMode(joystickSwitchPin, INPUT_PULLUP);

  lc.shutdown(0, false);
  lc.clearDisplay(0);

  pinMode(lcdBrightnessPin, OUTPUT);
  pinMode(lcdContrastPin, OUTPUT);
  analogWrite(lcdBrightnessPin, 128);
  analogWrite(lcdContrastPin, 145);
  lcd.begin(16, 2);
  lcd.clear();
  lcdMenu.greetingsShownTime = millis();
  lcd.createChar(lcdMenu.bombCharIndex, bombChar);
}

void loop() {
  applyConfigurationSettings();
  joystickManager.handleInput();

  if (gameState == IN_MENU) {
    lcdMenu.displayGreetingsOnLCD();
    if (lcdMenu.displayMenu) {
      lcdMenu.displayLCDMenu();
    }
    displayLoadingStateOnMatrix();
    handleStartGameChange();
  } else if (gameState == PLAYING) {
    resetGame();
    calculateElapsedTime();
    lcdMenu.displayGameInfo(bombsUsed, elapsedTime);
    handlePlayerMovement();
    displayBoardOnMatrix();
    handleBombPlacement();
    explodeBomb();
    checkGameWon();
    resetGame();
    resetGameOnJoystickLongPress();
  } else {
    handleResetGame(); 
    handleJoystickPressEndGame();
  }
}

void resetGameOnJoystickLongPress() {
  if (joystickManager.isLongPressed()) {
    gameState = RESET;
  }
}

void handleStartGameChange() {
  if (lcdMenu.canStartGame) {
    gameState = PLAYING;
  }
}

void handleJoystickPressEndGame() {
  if (joystickManager.isPressed()) {
    waitingForUserInputEndGame = false;
    delay(200); // add a small delay before taking the user to the menu
  }
}

void calculateElapsedTime() {
  if (millis() - lastSavedElapsedTime > oneSecondInMs) {
    elapsedTime += 1;
    lastSavedElapsedTime = millis();
  }
}

void applyConfigurationSettings() {
  analogWrite(lcdBrightnessPin, eepromManager.getLcdBrightness());
  analogWrite(lcdContrastPin, eepromManager.getLcdContrast());
  lc.setIntensity(0, eepromManager.getMatrixBrightness());
}

void handleBombPlacement() {
  if (millis() - lastSavedElapsedTime < startDelayForBombPlacement) {
    return;
  }

  if (!joystickManager.isPressed() || bombPlaced) {
    return;
  }

  board[playerPositionRow][playerPositionCol] = bomb;
  bombPlacementTime = millis();
  bombPlaced = true;
  bombsUsed += 1;
}

void explodeBomb() {
  if (!bombPlaced || millis() - bombPlacementTime < bombExplosionTime) {
    return;
  }

  unsigned long detonationTime = bombPlacementTime + bombExplosionTime;
  bool animate = millis() - detonationTime < explodeAnimationTime;

  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      if (board[i][j] == bomb) {
        bombPositionRow = i;
        bombPositionCol = j;
      }
    }
  }
  destroyArea(bombPositionRow, bombPositionCol, animate);
}

// first animate the bomb going off and then destroy the walls
void destroyArea(int row, int col, bool animate) {
  updateNeighborsBomb(row, col, animate);
  if (!animate) {
    bombPlaced = false;
  }
}

void updateNeighborsBomb(int row, int col, bool animate) {
  int newValue = animate ? 1 : 0;
  const int animationLen = 5;
  int directionsDestroyAnimation[animationLen][2] = { { 0, 0 }, { 0, -1 }, { 0, 1 }, { -1, 0 }, { 1, 0 } };

  for (int i = 0; i < animationLen; ++i) {
    int newRow = row + directionsDestroyAnimation[i][0];
    int newCol = col + directionsDestroyAnimation[i][1];
    if (newRow >= 0 && newRow < boardSize && newCol >= 0 && newCol < boardSize) {
      board[newRow][newCol] = newValue;
      if (!animate && playerPositionRow == newRow && playerPositionCol == newCol) {
        gameState = LOST;
      }
    }
  }
}

void displayBoardOnMatrix() {
  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      if (board[i][j] == wall) {
        lc.setLed(0, i, j, true);
      } else if (board[i][j] == bomb) {
        blinkMatrixLed(i, j, lastBombBlinkTime, fastBlinkTime);
      } else if (i == playerPositionRow && j == playerPositionCol) {
        blinkMatrixLed(i, j, lastPlayerBlinkTime, slowBlinkTime);
      } else if (board[i][j] == empty) {
        lc.setLed(0, i, j, false);
      }
    }
  }
}

void handlePlayerMovement() {
  if (joystickManager.getMovementDirection() == NONE) {
    return;
  }

  switch (joystickManager.getMovementDirection()) {
    case LEFT:
      if (playerPositionCol == 0 || board[playerPositionRow][playerPositionCol - 1]) {
        return;
      }
      playerPositionCol -= 1;
      break;
    case RIGHT:
      if (playerPositionCol == boardSize - 1 || board[playerPositionRow][playerPositionCol + 1]) {
        return;
      }
      playerPositionCol += 1;
      break;
    case DOWN:
      if (playerPositionRow == boardSize - 1 || board[playerPositionRow + 1][playerPositionCol]) {
        return;
      }
      playerPositionRow += 1;
      break;
    case UP:
      if (playerPositionRow == 0 || board[playerPositionRow - 1][playerPositionCol]) {
        return;
      }
      playerPositionRow -= 1;
      break;
  }
}

void checkGameWon() {
  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      if (board[i][j] == wall) {
        return;
      }
    }
  }
  gameState = WON;
}

void handleResetGame() {
  if (gameState == WON) {
    printByte(smile);
    lcdMenu.displayEndGameMessage(endGameMessages[0]);
  } else if (gameState == LOST) {
    printByte(sad);
    lcdMenu.displayEndGameMessage(endGameMessages[1]);
  }

  if (gameState != RESET && waitingForUserInputEndGame) {
    return;
  }

  shouldResetGame = true;
  lcdMenu.canStartGame = false;
  lcdMenu.displayIntro = true;
  gameState = IN_MENU;
}

void resetGame() {
  if (!shouldResetGame) {
    return;
  }

  shouldResetGame = false;
  bombPlaced = false;
  bombPlacementTime = 0;
  bombPositionRow = -1;
  bombPositionCol = -1;
  playerPositionRow = random(boardSize);
  playerPositionCol = random(boardSize);
  bombsUsed = 0;
  elapsedTime = 0;
  waitingForUserInputEndGame = true;

  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      // choosing the 0s and 1s on the board based on the difficulty
      int randomNum = random(maxDifficulty);
      if (randomNum < 3 * difficulty) {
        board[i][j] = 1;
      } else {
        board[i][j] = 0;
      }
    }
  }

  // leave some empty spaces near the player
  board[playerPositionRow][playerPositionCol] = 0;
  if (playerPositionRow > 0) {
    board[playerPositionRow - 1][playerPositionCol] = 0;
  }
  if (playerPositionRow < boardSize - 1) {
    board[playerPositionRow + 1][playerPositionCol] = 0;
  }
  if (playerPositionCol > 0) {
    board[playerPositionRow][playerPositionCol - 1] = 0;
  }
  if (playerPositionCol < boardSize - 1) {
    board[playerPositionRow][playerPositionCol + 1] = 0;
  }
}

void blinkMatrixLed(int row, int col, unsigned long& lastBlinkTime, int blinkDuration) {
  if (millis() - lastBlinkTime < blinkDuration) {
    return;
  }
  if (millis() - lastBlinkTime < 2 * blinkDuration) {
    lc.setLed(0, row, col, true);
    return;
  }
  lc.setLed(0, row, col, false);
  lastBlinkTime = millis();
}

// this function and the emojis are from here:
// https://techatronic.com/facial-expression-on-8x8-led-matrix-using-arduino/
void printByte(byte character[]) {
  int i = 0;
  for (i = 0; i < 8; i++) {
    lc.setRow(0, i, character[i]);
  }
}

void displayLoadingStateOnMatrix() {
  if (millis() - lastLoadingImageDisplayTime < loadingImageChangeTime) {
    return;
  }
  displayImage(loadingStateImages[lastImgIdx]);
  if (++lastImgIdx >= imagesLen) {
    lastImgIdx = 0;
  }
  lastLoadingImageDisplayTime = millis();
}

// https://xantorohara.github.io/led-matrix-editor/#
void displayImage(uint64_t image) {
  for (int i = 0; i < 8; i++) {
    byte row = (image >> i * 8) & 0xFF;
    for (int j = 0; j < 8; j++) {
      lc.setLed(0, i, j, bitRead(row, j));
    }
  }
}
