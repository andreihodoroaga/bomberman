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


