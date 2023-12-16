#ifndef STORAGE_H
#define STORAGE_H

#include "EEPROM.h"

class Storage {
public:
  const int howToPlayStartIndex = 6;
  const int howToPlayEndIndex = 114; 
  const int playerNameStartIndex = 115;
  const int playerNameEndIndex = 120;
  const int lcdBrightnessIndex = 0;
  const int lcdContrastIndex = 2;
  const int matrixBrightnessIndex = 4;
  const int howToPlaySize = howToPlayEndIndex - howToPlayStartIndex + 2; // the +2 includes the null character
  const int playerNameSize = playerNameEndIndex - playerNameStartIndex + 2;

  int getValueAtIndex(int index);
  void updateSettingsValue(int updateValue, int minValue, int maxValue, int eepromIndex);
  int getLcdBrightness();
  int getLcdContrast();
  int getMatrixBrightness();
  void writeString(int addr, const char* str);
  const char* getString(int startIndex, int endIndex, char* buffer);
  const char* getHowToPlayMessage(char* buffer);
  const char* getPlayerName(char* buffer);
  void updatePlayerNameCharacter(int nameIndex, char newChr);
  char getPlayerNameCharacter(int index);
};

#endif