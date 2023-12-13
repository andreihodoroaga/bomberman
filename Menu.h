#ifndef MENU_H
#define MENU_H

#include "LiquidCrystal.h"
#include "Arduino.h"
#include "JoystickManager.h"
#include "EEPROMManager.h"

class Menu {
private:
  LiquidCrystal& lcd;
  JoystickManager& joystickManager;
  EEPROMManager& eepromManager;

  const int minLcdBrightness = 0;
  const int maxLcdBrightness = 200;
  const int minMatrixBrightness = 0;
  const int maxMatrixBrightness = 15;
  const int lcdUpdateStep = 5;
  const int matrixUpdateStep = 1;
  const int displayIntroMessageTime = 1000;
  const int lcdRows = 2;
  const int lcdCols = 16;
  const int brightnessNumberColIdx = 9;
  const int endGameMessageUpdateRate = 500;

  const char* greetingsTexts[2] = { "Welcome to", "Bomberman!" };
  const char* menuOptions[4] = {
    "Start", "About", "Settings", "High Score"
  };
  const char* menuAboutSection[4] = {
    "Bomberman",
    "By Andrei Hodoroaga",
    "https://github.com/andreihodoroaga/introductiontorobotics",
    "Back"
  };
  const char* menuSettingsSection[2] = {
    "Brightness",
    "Back"
  };
  const char* brightnessSettings[3] = {
    "LCD",
    "Matrix",
    "Back"
  };
  const char* gameInfoTexts[2] = { "Bombs used: ", "Elapsed time: " };
  const char* endGameContinueText = "Press on the joystick to continue...";

  int oldBombsUsed = -1;
  int oldElapsedTime = -1;
  unsigned long lastMenuPress = 0;
  bool canEnterBrightness = false;
  bool displayGreetings = true;
  bool displayBomb = false;
  int currMenuBombCol = 0;
  int currMenuBombRow = 0;
  int currMenuOption = 0;
  int currMenuHeight = 4;  // TODO: update this to be dynamic for each section
  int aboutSectionNameStartIndex = 0;
  int aboutSectionGitLinkStartIndex = 0;
  int lastEndGameLetterIdx = 0;
  unsigned long lastEndGameLetterChange = 0;

  void displayTextOnLCD(const char* text, int textStartIndex, int col, int line);
  void displayCurrentMenuOptions();
  void handleJoystickMenuPress();
  void resetMenuOptions();
  void handleJoystickMenuChange();
  void handleJoystickBrightnessUpdates(int updateValue);
  void addBrightnessValueOnDisplay();
public:
  unsigned long greetingsShownTime = 0;
  bool displayMenu = false;
  bool displayIntro = true;
  bool displayMenuOptions = false;
  int currMenuSubOption = -1;
  const int bombCharIndex = 0;
  bool canStartGame = false;

  Menu(LiquidCrystal& lcdObj, JoystickManager& joystickManagerObj, EEPROMManager& eepromManagerObj)
    : lcd(lcdObj), joystickManager(joystickManagerObj), eepromManager(eepromManagerObj) {}
  void displayGreetingsOnLCD();
  void displayLCDMenu();
  void displayGameInfo(int bombsUsed, unsigned long elapsedTime);
  void displayEndGameMessage(const char* text);
};

#endif
