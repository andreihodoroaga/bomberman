#include "EEPROM.h"
#include "EEPROMManager.h"
#include "Arduino.h"

int EEPROMManager::getValueAtIndex(int index) {
  int value;
  EEPROM.get(index, value);
  return value;
}

void EEPROMManager::updateSettingsValue(int updateValue, int minValue, int maxValue, int eepromIndex) {
  int currentValue = getValueAtIndex(eepromIndex);
  int value = currentValue + updateValue;
  if (value < minValue || value > maxValue) {
    value = currentValue;
  }
  EEPROM.put(eepromIndex, value);
}

int EEPROMManager::getLcdBrightness() {
  return getValueAtIndex(lcdBrightnessIndex);
}

int EEPROMManager::getLcdContrast() {
  return getValueAtIndex(lcdContrastIndex);
}

int EEPROMManager::getMatrixBrightness() {
  return getValueAtIndex(matrixBrightnessIndex);
}

const char* EEPROMManager::getHowToPlayMessage(char* buffer) {
  return getString(howToPlayStartIndex, howToPlayEndIndex, buffer);
}

const char* EEPROMManager::getPlayerName(char* buffer) {
  return getString(playerNameStartIndex, playerNameEndIndex, buffer);
}

const char* EEPROMManager::getString(const int startIndex, const int endIndex, char* buffer) {
  const int length = endIndex - startIndex + 1;
  for (int i = 0; i < length; i++) {
    buffer[i] = EEPROM.read(startIndex + i);
  }
  buffer[length] = '\0';
  return buffer;
}

void EEPROMManager::writeString(int addr, const char* str) {
  int length = strlen(str);

  for (int i = 0; i < length; i++) {
    EEPROM.update(addr + i, str[i]);
  }
}

void EEPROMManager::updatePlayerNameCharacter(int nameIndex, char newChr) {
  int eepromIndex = playerNameStartIndex + nameIndex;
  EEPROM.update(eepromIndex, newChr);
}

char EEPROMManager::getPlayerNameCharacter(int index) {
  return EEPROM.read(playerNameStartIndex + index);
}
