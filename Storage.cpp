#include "EEPROM.h"
#include "Storage.h"

int Storage::getIntValueAtIndex(int index) {
  int value;
  EEPROM.get(index, value);
  return value;
}

byte Storage::getByteValueAtIndex(int index) {
  return EEPROM.read(index);
}

void Storage::updateValueAtIndex(int index, byte value) {
  EEPROM.put(index, value);
}

void Storage::updateSettingsValue(int updateValue, int minValue, int maxValue, int eepromIndex) {
  int currentValue = getIntValueAtIndex(eepromIndex);
  int value = currentValue + updateValue;
  Serial.println(value);
  if (value < minValue || value > maxValue) {
    value = currentValue;
  }
  EEPROM.put(eepromIndex, value);
}

int Storage::getLcdBrightness() {
  return getIntValueAtIndex(lcdBrightnessIndex);
}

int Storage::getLcdContrast() {
  return getIntValueAtIndex(lcdContrastIndex);
}

int Storage::getMatrixBrightness() {
  return getIntValueAtIndex(matrixBrightnessIndex);
}

const char* Storage::getHowToPlayMessage(char* buffer) {
  return getString(howToPlayStartIndex, howToPlayEndIndex, buffer);
}

const char* Storage::getPlayerName(char* buffer) {
  return getString(playerNameStartIndex, playerNameEndIndex, buffer);
}

const char* Storage::getHighScorePlayerName(char* buffer, int index) {
  int startIndex = highScoreNamesStartIndex + index * (playerNameSize - 1); // -1 for the null character
  return getString(startIndex, startIndex + playerNameSize - 2, buffer);
}

void Storage::setHighScorePlayerName(const char* str, int index) {
  int addr = highScoreNamesStartIndex + index * (playerNameSize - 1);
  writeString(addr, str);
}

const char* Storage::getString(const int startIndex, const int endIndex, char* buffer) {
  const int length = endIndex - startIndex + 1;
  for (int i = 0; i < length; i++) {
    buffer[i] = EEPROM.read(startIndex + i);
  }
  buffer[length] = '\0';
  return buffer;
}

void Storage::writeString(int addr, const char* str) {
  int length = strlen(str);

  for (int i = 0; i < length; i++) {
    EEPROM.update(addr + i, str[i]);
  }
}

void Storage::updatePlayerNameCharacter(int nameIndex, char newChr) {
  int eepromIndex = playerNameStartIndex + nameIndex;
  EEPROM.update(eepromIndex, newChr);
}

char Storage::getPlayerNameCharacter(int index) {
  return EEPROM.read(playerNameStartIndex + index);
}

byte Storage::getBoard(int i, int j) {
  return getIntValueAtIndex(getBoardIndex(i, j));
}

void Storage::updateBoard(int i, int j, byte val) {
  EEPROM.put(getBoardIndex(i, j), val);
}

int Storage::getBoardIndex(int i, int j) {
  return boardStartIndex + boardSize * i + j;
}

int Storage::getHighScore(int index) {
  if (index >= numStoredHighScores) {
    return -1;
  }
  return getIntValueAtIndex(highScoreStartIndex + index * sizeof(index));
}

void Storage::setHighScore(int index, int value) {
  if (index >= numStoredHighScores) {
    return;
  }
  EEPROM.put(highScoreStartIndex + index * sizeof(index), value);
}

// Returns the current room, or 1 if it is not previously set in eeprom.
byte Storage::getRoom() {
  return getByteValueAtIndex(roomIndex) != 0 ? getByteValueAtIndex(roomIndex) : 1;
}

void Storage::setRoom(int value) {
  EEPROM.put(roomIndex, value);
}
