#ifndef STORAGE_H
#define STORAGE_H

#include "EEPROM.h"
#include "Arduino.h"

class Storage {
private:
  int getBoardIndex(int i, int j);
public:
  static int matrixWrites;  // TODO: delete?
  const int lcdBrightnessIndex = 0;
  const int lcdContrastIndex = 2;
  const int matrixBrightnessIndex = 4;
  const int howToPlayStartIndex = 6;
  const int howToPlayEndIndex = 114;
  const int playerNameStartIndex = 115;
  const int playerNameEndIndex = 120;
  const int boardStartIndex = 150;
  const int boardEndIndex = 404;
  const int soundsSettingIndex = 405;
  const int highScoreStartIndex = 406;
  static const int numStoredHighScores = 3;
  const int highScoreNamesStartIndex = 412;
  const int roomIndex = 425;
  const int boardSize = 16;
  const int howToPlaySize = howToPlayEndIndex - howToPlayStartIndex + 2;  // the +2 includes the null character
  const int playerNameSize = playerNameEndIndex - playerNameStartIndex + 2;
  const int defaultHighScoreValue = -1;

  int getIntValueAtIndex(int index);
  byte getByteValueAtIndex(int index);
  void updateValueAtIndex(int index, byte value);
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
  byte getBoard(int i, int j);
  void updateBoard(int i, int j, byte val);
  int getHighScore(int index);
  void setHighScore(int index, int value);
  const char* getHighScorePlayerName(char* buffer, int index);
  void setHighScorePlayerName(const char* str, int index);
  byte getRoom();
  void setRoom(int value);
};

#endif
