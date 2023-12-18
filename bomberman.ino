#include "LedControl.h"
#include "Menu.h"
#include "LiquidCrystal.h"
#include "Joystick.h"
#include "Storage.h"
#include "Buzzer.h"

// Joystick
const int joystickXPin = A0;
const int joystickYPin = A1;
const int joystickSwitchPin = 2;
const bool joystickCommonAnode = false;

Joystick joystick(joystickXPin, joystickYPin, joystickSwitchPin, joystickCommonAnode);
Storage storage;
Buzzer buzzer;

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
int boardSize;
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
const int maxHighScoreAllowed = 9999;
const int oneSecondInMs = 1000;
bool waitingForUserInputEndGame = true;
const int displayedBoardSize = 8;
// The lower bounds of the visible board (<=8)
int displayedBoardStartRow = 0;
int displayedBoardStartCol = 0;
int roomNumbers[4] = { 1, 2, 3, 4 };
int bombRadius = 1;
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
Menu lcdMenu(lcd, joystick, storage);
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
byte resetHighScoresChar[] = {
  B00000,
  B00001,
  B00011,
  B10110,
  B11100,
  B01000,
  B00000,
  B00000
};
byte clockChar[] = {
  B00000,
  B01110,
  B10101,
  B10111,
  B10001,
  B01110,
  B00000,
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
  lcd.begin(16, 2);
  lcd.clear();
  lcdMenu.greetingsShownTime = millis();
  lcd.createChar(lcdMenu.bombCharIndex, bombChar);
  lcd.createChar(lcdMenu.resetHighScoresCharIndex, resetHighScoresChar);
  lcd.createChar(lcdMenu.clockCharIndex, clockChar);
  pinMode(Buzzer::pin, OUTPUT);
  boardSize = storage.boardSize;

  // storage.writeString(storage.howToPlayStartIndex, "Blast through the board, drop bombs by pressing the joystick, and dash away! Watch out, the boom gets bigger!");
  // storage.writeString(storage.playerNameStartIndex, "player");
}

void loop() {
  applyConfigurationSettings();
  joystick.handleInput();

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
    updateRoom();
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

  // if (buzzer.isPlaying) {
  //   buzzer.play(50, 1000);
  // }
}

void updateRoom() {
  byte currentRoom = storage.getRoom();
  if (displayedBoardStartRow == 0 && displayedBoardStartCol == 0) {
    currentRoom = roomNumbers[0];
  }
  if (displayedBoardStartRow == 0 && displayedBoardStartCol == displayedBoardSize) {
    currentRoom = roomNumbers[1];
  }
  if (displayedBoardStartRow == displayedBoardSize && displayedBoardStartCol == 0) {
    currentRoom = roomNumbers[2];
  }
  if (displayedBoardStartRow == displayedBoardSize && displayedBoardStartCol == displayedBoardSize) {
    currentRoom = roomNumbers[3];
  }
  storage.setRoom(currentRoom);

  if (!bombPlaced) {
    updateBombRadius(currentRoom);
  }  
}

void updateBombRadius(byte currentRoom) {
  if (currentRoom == roomNumbers[1] && checkClearedRoom(roomNumbers[0])) {
    bombRadius = 2;
  } else if ((currentRoom == roomNumbers[2] || currentRoom == roomNumbers[3]) && checkClearedRoom(roomNumbers[1])) {
    bombRadius = 3;
  } else {
    bombRadius = 1;
  }
}

bool checkClearedRoom(int roomNumber) {
  int roomStartRow = 0;
  int roomStartCol = 0;

  if (roomNumber == roomNumbers[1]) {
    roomStartCol = displayedBoardSize;
  } else if (roomNumber == roomNumbers[2]) {
    roomStartRow = displayedBoardSize;
  } else if (roomNumber == roomNumbers[3]) {
    roomStartCol = displayedBoardSize;
    roomStartRow = displayedBoardSize;
  }

  for (int i = roomStartRow; i < roomStartRow + displayedBoardSize; i++) {
    for (int j = roomStartCol; j < roomStartCol + displayedBoardSize; j++) {
      if (storage.getBoard(i, j) == wall) {
        return false;
      }
    }
  }

  return true;
}

void resetGameOnJoystickLongPress() {
  if (joystick.isLongPressed()) {
    gameState = RESET;
  }
}

void handleStartGameChange() {
  if (lcdMenu.canStartGame) {
    gameState = PLAYING;
  }
}

void handleJoystickPressEndGame() {
  if (joystick.isPressed()) {
    waitingForUserInputEndGame = false;
    delay(200);  // add a small delay before taking the user to the menu
  }
}

void calculateElapsedTime() {
  if (millis() - lastSavedElapsedTime > oneSecondInMs) {
    elapsedTime += 1;
    lastSavedElapsedTime = millis();
  }
}

void applyConfigurationSettings() {
  analogWrite(lcdBrightnessPin, storage.getLcdBrightness());
  analogWrite(lcdContrastPin, storage.getLcdContrast());
  lc.setIntensity(0, storage.getMatrixBrightness());
}

void handleBombPlacement() {
  if (millis() - lastSavedElapsedTime < startDelayForBombPlacement) {
    return;
  }

  if (!joystick.isPressed() || bombPlaced) {
    return;
  }

  storage.updateBoard(playerPositionRow, playerPositionCol, bomb);
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
      if (storage.getBoard(i, j) == bomb) {
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
  for (int i = -bombRadius; i <= bombRadius; i++) {
    bombCell(row + i, col, animate);
  }
  for (int j = -bombRadius; j <= bombRadius; ++j) {
    // already bombed the cell at (0, 0) in the previous for loop
    if (j != 0) {
      bombCell(row, col + j, animate);
    }
  }
}

// returns 1 if the bombed cell was a wall
void bombCell(int row, int col, int animate) {
  byte newValue = animate ? 1 : 0;

  if (row >= 0 && row < boardSize && col >= 0 && col < boardSize) {
    storage.updateBoard(row, col, newValue);
    if (!animate && playerPositionRow == row && playerPositionCol == col) {
      gameState = LOST;
    }
  }
}

void displayBoardOnMatrix() {
  for (int i = displayedBoardStartRow; i < displayedBoardSize + displayedBoardStartRow; i++) {
    for (int j = displayedBoardStartCol; j < displayedBoardSize + displayedBoardStartCol; j++) {
      int matrixI = i - displayedBoardStartRow;
      int matrixJ = j - displayedBoardStartCol;

      if (storage.getBoard(i, j) == wall) {
        lc.setLed(0, matrixI, matrixJ, true);
      } else if (storage.getBoard(i, j) == bomb) {
        blinkMatrixLed(matrixI, matrixJ, lastBombBlinkTime, fastBlinkTime);
      } else if (i == playerPositionRow && j == playerPositionCol) {
        blinkMatrixLed(matrixI, matrixJ, lastPlayerBlinkTime, slowBlinkTime);
      } else if (storage.getBoard(i, j) == empty) {
        lc.setLed(0, matrixI, matrixJ, false);
      }
    }
  }
}

void handlePlayerMovement() {
  if (joystick.getMovementDirection() == NONE) {
    return;
  }

  buzzer.isPlaying = true;
  buzzer.lastBuzzerPlayTime = millis();
  lastPlayerBlinkTime = millis();
  switch (joystick.getMovementDirection()) {
    case LEFT:
      if (playerPositionCol == 0 || storage.getBoard(playerPositionRow, playerPositionCol - 1)) {
        return;
      }
      playerPositionCol -= 1;
      if (playerPositionCol == displayedBoardSize - 1) {
        displayedBoardStartCol = 0;
      }
      break;
    case RIGHT:
      if (playerPositionCol == boardSize - 1 || storage.getBoard(playerPositionRow, playerPositionCol + 1)) {
        return;
      }
      playerPositionCol += 1;
      if (playerPositionCol == displayedBoardSize) {
        displayedBoardStartCol = displayedBoardSize;
      }
      break;
    case DOWN:
      if (playerPositionRow == boardSize - 1 || storage.getBoard(playerPositionRow + 1, playerPositionCol)) {
        return;
      }
      playerPositionRow += 1;
      if (playerPositionRow == displayedBoardSize) {
        displayedBoardStartRow = displayedBoardSize;
      }
      break;
    case UP:
      if (playerPositionRow == 0 || storage.getBoard(playerPositionRow - 1, playerPositionCol)) {
        return;
      }
      playerPositionRow -= 1;
      if (playerPositionRow == displayedBoardSize - 1) {
        displayedBoardStartRow = 0;
      }
      break;
  }
}

void checkGameWon() {
  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      if (storage.getBoard(i, j) == wall) {
        return;
      }
    }
  }
  gameState = WON;
  updateHighScores();
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
  displayedBoardStartRow = 0;
  displayedBoardStartCol = 0;
  playerPositionRow = random(displayedBoardSize);
  playerPositionCol = random(displayedBoardSize);
  bombsUsed = 0;
  elapsedTime = 0;
  waitingForUserInputEndGame = true;

  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      // choosing the 0s and 1s on the board based on the difficulty
      int randomNum = random(maxDifficulty);
      if (randomNum < 3 * difficulty) {
        storage.updateBoard(i, j, 1);
      } else {
        storage.updateBoard(i, j, 0);
      }
    }
  }

  // leave some empty spaces near the player
  storage.updateBoard(playerPositionRow, playerPositionCol, 1);
  if (playerPositionRow > 0) {
    storage.updateBoard(playerPositionRow, playerPositionCol, 0);
  }
  if (playerPositionRow < boardSize - 1) {
    storage.updateBoard(playerPositionRow + 1, playerPositionCol, 0);
  }
  if (playerPositionCol > 0) {
    storage.updateBoard(playerPositionRow, playerPositionCol - 1, 0);
  }
  if (playerPositionCol < boardSize - 1) {
    storage.updateBoard(playerPositionRow, playerPositionCol + 1, 0);
  }
}

void blinkMatrixLed(int row, int col, unsigned long& lastBlinkTime, int blinkDuration) {
  if (millis() - lastBlinkTime < blinkDuration) {
    lc.setLed(0, row, col, true);
    return;
  }
  if (millis() - lastBlinkTime < 2 * blinkDuration) {
    lc.setLed(0, row, col, false);
    return;
  }
  lastBlinkTime = millis();
}

void updateHighScores() {
  char currentPlayer[storage.playerNameSize];
  storage.getPlayerName(currentPlayer);
  unsigned long currentScore = elapsedTime <= maxHighScoreAllowed ? elapsedTime : maxHighScoreAllowed;
  int newScoreIdx = -1;
  for (int i = 0; i < storage.numStoredHighScores; i++) {
    int currHighScore = storage.getHighScore(i);
    if (currentScore < currHighScore || currHighScore == storage.defaultHighScoreValue) {
      newScoreIdx = i;
      break;
    }
  }
  if (newScoreIdx == -1) {
    return;
  }
  storage.setHighScore(newScoreIdx, currentScore);
  storage.setHighScorePlayerName(currentPlayer, newScoreIdx);
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

// TODO: Remove
void printBoard() {
  for (int i = 0; i < boardSize; i++) {
    for (int j = 0; j < boardSize; j++) {
      Serial.print(storage.getBoard(i, j));
      Serial.print(" ");
    }
    Serial.println();
  }
}

void printDisplayedBoard() {
  for (int i = displayedBoardStartRow; i < displayedBoardStartRow + displayedBoardSize; i++) {
    for (int j = displayedBoardStartCol; j < displayedBoardStartCol + displayedBoardSize; j++) {
      Serial.print(storage.getBoard(i, j));
      Serial.print(" ");
    }
    Serial.println();
  }
}
