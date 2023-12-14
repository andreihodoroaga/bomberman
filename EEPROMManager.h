#ifndef EEPROM_HANDLER_H
#define EEPROM_HANDLER_H

#include "EEPROM.h"

class EEPROMManager {
public:
  const int lcdBrightnessIndex = 0;
  const int lcdContrastIndex = 2;
  const int matrixBrightnessIndex = 4;

  int getValueAtIndex(int index);
  void updateSettingsValue(int updateValue, int minValue, int maxValue, int eepromIndex);
  int getLcdBrightness();
  int getLcdContrast();
  int getMatrixBrightness();
};

#endif
