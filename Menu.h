#ifndef MENU_H
#define MENU_H

#include "LiquidCrystal.h"
#include "Arduino.h"
#include "JoystickManager.h"
#include "EEPROMManager.h"

struct MenuItem {
  char* label;
  MenuItem* parent;
  MenuItem* firstChild;
  MenuItem* nextSibling;

  void addFirstChild(MenuItem* menuItem) {
    this->firstChild = menuItem;
  }
};

class Menu {
private:
  LiquidCrystal& lcd;
  JoystickManager& joystickManager;
  EEPROMManager& eepromManager;

  const int minLcdBrightness = 0;
  const int maxLcdBrightness = 200;
  const int minLcdContrast = 0;
  const int maxLcdContrast = 200;
  const int minMatrixBrightness = 0;
  const int maxMatrixBrightness = 15;
  const int lcdBrightnessUpdateStep = 5;
  const int lcdContrastUpdateStep = 5;
  const int matrixBrightnessUpdateStep = 1;
  const int displayIntroMessageTime = 1000;
  const int lcdRows = 2;
  const int lcdCols = 16;
  const int brightnessNumberColIdx = 9;
  const int endGameMessageUpdateRate = 500;
  const int scrollMenuStep = 5;

  char* backSectionName = "Back";
  char* aboutSectionName = "About";
  char* startSectionName = "Start";
  char* brightnessSectionName = "Brightness";
  char* contrastSectionName = "Contrast";
  char* lcdSectionName = "LCD";
  char* matrixSectionName = "Matrix";
  char* githubLink = "https://github.com/andreihodoroaga/introductiontorobotics";
  char* creatorName = "By Andrei Hodoroaga";

  MenuItem mainMenu = { "Main menu", NULL, NULL, NULL };
  MenuItem menuBack = { backSectionName, NULL, NULL, NULL };

  MenuItem menuHighScore = { "High Score", &mainMenu, NULL, NULL };
  MenuItem menuSettings = { "Settings", &mainMenu, NULL, &menuHighScore };
  MenuItem menuAbout = { aboutSectionName, &mainMenu, NULL, &menuSettings };
  MenuItem menuStart = { startSectionName, &mainMenu, NULL, &menuAbout };

  MenuItem highScoreThree = { "3. ", &menuHighScore, NULL, &menuBack };
  MenuItem highScoreTwo = { "2. ", &menuHighScore, NULL, &highScoreThree };
  MenuItem highScoreOne = { "1. ", &menuHighScore, NULL, &highScoreTwo };

  MenuItem settingsName = { "Name", &menuSettings, NULL, &menuBack };
  MenuItem settingsSounds = { "Sounds", &menuSettings, NULL, &settingsName };
  MenuItem settingsBrightness = { brightnessSectionName, &menuSettings, NULL, &settingsSounds };
  MenuItem settingsContrast = { contrastSectionName, &menuSettings, NULL, &settingsBrightness };

  MenuItem namePlayer = { "Name: ", &settingsName, NULL, &menuBack };

  MenuItem soundsBack = { "Back", &settingsSounds, NULL, NULL };

  MenuItem brightnessLCD = { lcdSectionName, &settingsBrightness, NULL, &menuBack };
  MenuItem brightnessMatrix = { "Matrix", &settingsBrightness, NULL, &brightnessLCD };

  MenuItem contrastLCD = { lcdSectionName, &settingsContrast, NULL, &menuBack };

  MenuItem aboutGithubLink = { githubLink, &menuAbout, NULL, &menuBack };
  MenuItem aboutCreatorName = { creatorName, &menuAbout, NULL, &aboutGithubLink };
  MenuItem aboutGameName = { "Bomberman", &menuAbout, NULL, &aboutCreatorName };

  MenuItem* currentSubMenu = &mainMenu;

  const char* greetingsTexts[2] = { "Welcome to", "Bomberman!" };
  const char* gameInfoTexts[2] = { "Bombs used: ", "Elapsed time: " };
  const char* endGameContinueText = "Press on the joystick to continue...";

  int oldBombsUsed = -1;
  int oldElapsedTime = -1;
  unsigned long lastMenuPress = 0;
  bool displayGreetings = true;
  bool displayBomb = false;
  int currMenuBombRow = 0;
  int aboutSectionNameStartIndex = 0;
  int aboutSectionGitLinkStartIndex = 0;
  int lastEndGameLetterIdx = 0;
  unsigned long lastEndGameLetterChange = 0;

  void displayTextOnLCD(const char* text, int textStartIndex, int col, int line);
  void displayCurrentMenuOptions();
  void handleJoystickMenuPress();
  void resetMenuOptions();
  void handleJoystickMenuChange();
  void handleBrightnessAndContrastUpdates(int updateValue);
  void displaySettingsValue(int row, MenuItem* subOption);
  MenuItem* getNthSubOption(int n);
  int getCurrentSubMenuSize();
  int getSettingEepromValue(char* mainCategory, char* subCategory);
  int getTextStartIndex(char* label);
  void handleScrollText(int valueMultiplier);
  void updateMenuScrollTextIndex(int& index, int valueMultiplier, char* sectionName);
public:
  unsigned long greetingsShownTime = 0;
  bool displayMenu = false;
  bool displayIntro = true;
  bool displayMenuOptions = false;
  const int bombCharIndex = 0;
  bool canStartGame = false;

  Menu(LiquidCrystal& lcdObj, JoystickManager& joystickManagerObj, EEPROMManager& eepromManagerObj);
  void displayGreetingsOnLCD();
  void displayLCDMenu();
  void displayGameInfo(int bombsUsed, unsigned long elapsedTime);
  void displayEndGameMessage(const char* text);
};

#endif
