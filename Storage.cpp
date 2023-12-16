#include "EEPROM.h"
#include "Storage.h"
#include "Arduino.h"

int Storage::getValueAtIndex(int index) {
  int value;
  EEPROM.get(index, value);
  return value;
}

void Storage::updateSettingsValue(int updateValue, int minValue, int maxValue, int eepromIndex) {
  int currentValue = getValueAtIndex(eepromIndex);
  int value = currentValue + updateValue;
  if (value < minValue || value > maxValue) {
    value = currentValue;
  }
  EEPROM.put(eepromIndex, value);
}

int Storage::getLcdBrightness() {
  return getValueAtIndex(lcdBrightnessIndex);
}

int Storage::getLcdContrast() {
  return getValueAtIndex(lcdContrastIndex);
}

int Storage::getMatrixBrightness() {
  return getValueAtIndex(matrixBrightnessIndex);
}

const char* Storage::getHowToPlayMessage(char* buffer) {
  return getString(howToPlayStartIndex, howToPlayEndIndex, buffer);
}

const char* Storage::getPlayerName(char* buffer) {
  return getString(playerNameStartIndex, playerNameEndIndex, buffer);
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
