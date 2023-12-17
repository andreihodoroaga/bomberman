#ifndef MENU_H
#define MENU_H

#include "LiquidCrystal.h"
#include "Arduino.h"
#include "Joystick.h"
#include "Storage.h"

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
  Joystick& joystick;
  Storage& storage;

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
  const int menuPressDelay = 400;

  const char* backSectionName = "Back";
  const char* aboutSectionName = "About";
  const char* startSectionName = "Start";
  const char* brightnessSectionName = "Brightness";
  const char* contrastSectionName = "Contrast";
  const char* lcdSectionName = "LCD";
  const char* matrixSectionName = "Matrix";
  const char* howToPlaySectionName = "How to play";
  const char* nameSection = "Name";
  const char* playerNameSection = "Name:";
  const char* githubLink = "https://github.com/andreihodoroaga/bomberman";
  const char* creatorName = "By Andrei Hodoroaga";
  const char* soundSectionLabel = "Sounds";
  const char* highScoreSection = "High Score";
  const char* highScoreLabels[Storage::numStoredHighScores] = { "1.", "2.", "3." };

  MenuItem mainMenu = { "Main menu", NULL, NULL, NULL };
  MenuItem menuBack = { backSectionName, NULL, NULL, NULL };
  MenuItem* currentSubMenu = &mainMenu;

  MenuItem menuHowToPlay = { howToPlaySectionName, &mainMenu, NULL, NULL };
  MenuItem menuHighScore = { highScoreSection, &mainMenu, NULL, &menuHowToPlay };
  MenuItem menuSettings = { "Settings", &mainMenu, NULL, &menuHighScore };
  MenuItem menuAbout = { aboutSectionName, &mainMenu, NULL, &menuSettings };
  MenuItem menuStart = { startSectionName, &mainMenu, NULL, &menuAbout };
  MenuItem highScoreThree = { highScoreLabels[2], &menuHighScore, NULL, &menuBack };
  MenuItem highScoreTwo = { highScoreLabels[1], &menuHighScore, NULL, &highScoreThree };
  MenuItem highScoreOne = { highScoreLabels[0], &menuHighScore, NULL, &highScoreTwo };
  MenuItem settingsResetHighScores = { "Reset scores", &menuSettings, NULL, &menuBack };
  MenuItem settingsContrast = { contrastSectionName, &menuSettings, NULL, &settingsResetHighScores };
  MenuItem settingsBrightness = { brightnessSectionName, &menuSettings, NULL, &settingsContrast };
  MenuItem settingsSounds = { soundSectionLabel, &menuSettings, NULL, &settingsBrightness };
  MenuItem settingsName = { nameSection, &menuSettings, NULL, &settingsSounds };
  MenuItem namePlayer = { playerNameSection, &settingsName, NULL, &menuBack };
  MenuItem soundsSetting = { "", &settingsSounds, NULL, &menuBack };
  MenuItem howToPlay = { "", &menuHowToPlay, NULL, &menuBack };
  MenuItem brightnessLCD = { lcdSectionName, &settingsBrightness, NULL, &menuBack };
  MenuItem brightnessMatrix = { "Matrix", &settingsBrightness, NULL, &brightnessLCD };
  MenuItem contrastLCD = { lcdSectionName, &settingsContrast, NULL, &menuBack };
  MenuItem aboutGithubLink = { githubLink, &menuAbout, NULL, &menuBack };
  MenuItem aboutCreatorName = { creatorName, &menuAbout, NULL, &aboutGithubLink };
  MenuItem aboutGameName = { "Bomberman", &menuAbout, NULL, &aboutCreatorName };

  const char* soundOnLabel = "On";
  const char* soundOffLabel = "Off";
  const char* greetingsTexts[2] = { "Welcome to", "Bomberman!" };
  const char* gameInfoTexts[2] = { "Used: ", "Room: " };
  const char* endGameContinueText = "Press on the joystick to continue...";

  int oldBombsUsed = -1;
  int oldElapsedTime = -1;
  unsigned long lastMenuPress = 0;
  bool displayGreetings = true;
  bool displayBomb = false;
  int currMenuBombRow = 0;
  int aboutSectionNameStartIndex = 0;
  int aboutSectionGitLinkStartIndex = 0;
  int howToPlaySectionStartIndex = 0;
  int lastEndGameLetterIdx = 0;
  unsigned long lastEndGameLetterChange = 0;
  int currentNameIdx = 0;
  bool isEditingName = false;
  unsigned long lastNameLetterBlink = 0;
  const int nameLetterBlinkDelay = 500;
  const int maxSettingsValueLength = 12;
  unsigned long resetScoresIconShowTime = 0;
  const int showResetScoresIconTime = 1000;
  bool showResetIcon = false;
  const int maxScoreDigits = 4;
  const int resetScoresIconCol = 14;
  const int resetScoresIconRow = 0;
  const int clockIconDisplayCol = 11;
  const int clockIconDisplayRow = 0;

  void displayTextOnLCD(const char* text, int textStartIndex, int col, int line);
  void displayCurrentMenuOptions();
  void handleJoystickMenuPress();
  void resetMenuOptions();
  void handleJoystickMenuChange();
  void handleBrightnessAndContrastUpdates(int updateValue);
  void displaySettingsValue(int row, MenuItem* subOption);
  MenuItem* getNthSubOption(int n);
  int getCurrentSubMenuSize();
  void getSettingStorageValue(char* mainCategory, char* subCategory, char* buffer);
  int getTextStartIndex(char* label);
  void handleScrollText(int valueMultiplier);
  void updateMenuScrollTextIndex(int& index, int valueMultiplier, int sectionNameLength);
  void handlePlayerNameEdit(int direction);
  void handlePlayerNameChange();
  void handleNameLetterChange(int direction);
  char* getSoundsSetting();
  void getHighScoreForPlayer(char* buffer, int index);
  void resetHighScores();
  void showResetScoresIcon();
public:
  unsigned long greetingsShownTime = 0;
  bool displayMenu = false;
  bool displayIntro = true;
  bool displayMenuOptions = false;
  const int bombCharIndex = 0;
  const int resetHighScoresCharIndex = 1;
  const int clockCharIndex = 2;
  bool canStartGame = false;

  Menu(LiquidCrystal& lcdObj, Joystick& joystickObj, Storage& storageObj);
  void displayGreetingsOnLCD();
  void displayLCDMenu();
  void displayGameInfo(int bombsUsed, unsigned long elapsedTime);
  void displayEndGameMessage(const char* text);
};

#endif
