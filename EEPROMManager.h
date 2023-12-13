#ifndef EEPROM_HANDLER_H
#define EEPROM_HANDLER_H

#include "EEPROM.h"

class EEPROMManager {
private:
  const int lcdBrightnessIndex = 0;
  const int matrixBrightnessIndex = 2;

  void updateBrightness(int updateValue, int minValue, int maxValue, int currentValue, int eepromIndex);

public:
  int getLcdBrightness();
  int getMatrixBrightness();
  void updateLCDBrightness(int updateValue, int minValue, int maxValue);
  void updateMatrixBrightness(int updateValue, int minValue, int maxValue);
};

#endif
