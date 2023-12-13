#include "EEPROMManager.h"
#include "Arduino.h"

int EEPROMManager::getLcdBrightness() {
  int brightness;
  EEPROM.get(lcdBrightnessIndex, brightness);
  return brightness;
}

int EEPROMManager::getMatrixBrightness() {
  int brightness;
  EEPROM.get(matrixBrightnessIndex, brightness);
  return brightness;
}

void EEPROMManager::updateLCDBrightness(int updateValue, int minValue, int maxValue) {
  updateBrightness(updateValue, minValue, maxValue, getLcdBrightness(), lcdBrightnessIndex);
}

void EEPROMManager::updateMatrixBrightness(int updateValue, int minValue, int maxValue) {
  updateBrightness(updateValue, minValue, maxValue, getMatrixBrightness(), matrixBrightnessIndex);
}

void EEPROMManager::updateBrightness(int updateValue, int minValue, int maxValue, int currentValue, int eepromIndex) {
  int value = currentValue + updateValue;
  if (value < minValue || value > maxValue) {
    value = currentValue;
  }
  EEPROM.put(eepromIndex, value);
}
