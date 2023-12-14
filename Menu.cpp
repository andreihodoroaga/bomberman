#include "Arduino.h"
#include "Menu.h"

Menu::Menu(LiquidCrystal& lcdObj, JoystickManager& joystickManagerObj, EEPROMManager& eepromManagerObj)
  : lcd(lcdObj), joystickManager(joystickManagerObj), eepromManager(eepromManagerObj) {
  mainMenu.addFirstChild(&menuStart);
  menuHighScore.addFirstChild(&highScoreOne);
  menuSettings.addFirstChild(&settingsContrast);
  menuAbout.addFirstChild(&aboutGameName);
  settingsName.addFirstChild(&namePlayer);
  settingsSounds.addFirstChild(&soundsBack);
  settingsBrightness.addFirstChild(&brightnessMatrix);
  settingsContrast.addFirstChild(&contrastLCD);
}

// Returns the nth sub option of the currentSubMenu, starting from 0.
MenuItem* Menu::getNthSubOption(int n) {
  MenuItem* currentSubOption = currentSubMenu->firstChild;
  for (int i = 0; i < n; i++) {
    if (currentSubOption->nextSibling != NULL) {
      currentSubOption = currentSubOption->nextSibling;
    } else {
      return NULL;
    }
  }
  return currentSubOption;
}

int Menu::getCurrentSubMenuSize() {
  int size = 0;
  MenuItem* currentSubOption = currentSubMenu->firstChild;
  while (currentSubOption != NULL) {
    currentSubOption = currentSubOption->nextSibling;
    size += 1;
  }
  return size;
}

void Menu::displayGreetingsOnLCD() {
  if (!displayIntro) {
    return;
  }

  if (displayGreetings) {
    displayTextOnLCD(greetingsTexts[0], 0, 0, 0);
    displayTextOnLCD(greetingsTexts[1], 0, 0, 1);
    displayGreetings = false;
  }
  if (millis() - greetingsShownTime > displayIntroMessageTime) {
    lcd.clear();
    displayMenu = true;
    displayBomb = true;
    displayMenuOptions = true;
    displayIntro = false;
  }
}

void Menu::displayLCDMenu() {
  if (displayBomb) {
    lcd.home();
    // % 2 because we always display only 2 rows
    lcd.setCursor(0, currMenuBombRow % 2);
    lcd.write(bombCharIndex);
    displayBomb = false;
  }
  if (displayMenuOptions) {
    displayCurrentMenuOptions();
    displayMenuOptions = false;
  }
  handleJoystickMenuChange();
  handleJoystickMenuPress();
}

void Menu::displayTextOnLCD(const char* text, int textStartIndex, int col, int line) {
  lcd.setCursor(col, line);

  int textLength = strlen(text);
  for (int i = 0; i < min(textLength, lcdCols - 1); i++) {
    int charIndex = textStartIndex + i;
    if (charIndex < textLength) {
      lcd.write(text[charIndex]);
    } else {
      lcd.write(' ');
    }
  }
}

void Menu::displayCurrentMenuOptions() {
  lcd.clear();
  displayBomb = true;

  MenuItem* currSubOption;
  MenuItem* nextSubOption;
  int currRow = currMenuBombRow % 2 == 0 ? currMenuBombRow : currMenuBombRow - 1;
  int nextRow = currMenuBombRow % 2 == 0 ? currMenuBombRow + 1 : currMenuBombRow;
  currSubOption = getNthSubOption(currRow);
  nextSubOption = getNthSubOption(nextRow);
  displayTextOnLCD(currSubOption->label, getTextStartIndex(currSubOption->label), 1, 0);
  displaySettingsValue(currRow, currSubOption);
  if (nextSubOption != NULL) {
    displayTextOnLCD(nextSubOption->label, getTextStartIndex(nextSubOption->label), 1, 1);
    displaySettingsValue(nextRow, nextSubOption);
  }
}

int Menu::getTextStartIndex(char* label) {
  if (strcmp(label, githubLink) == 0) {
    return aboutSectionGitLinkStartIndex;
  } 
  if (strcmp(label, creatorName) == 0) {
    return aboutSectionNameStartIndex;
  }
  return 0;
}

void Menu::displaySettingsValue(int row, MenuItem* subOption) {
  if (currentSubMenu->label != brightnessSectionName && currentSubMenu->label != contrastSectionName) {
    return;
  }

  int writeIdx = strlen(subOption->label) + 2; // +2 because of the bomb at index 0
  lcd.setCursor(writeIdx, row);
  int settingValue = getSettingEepromValue(currentSubMenu->label, subOption->label);
  if (settingValue == -1) {
    return;
  }
  lcd.print(settingValue);
}

int Menu::getSettingEepromValue(char* mainCategory, char* subCategory) {
  if (strcmp(mainCategory, brightnessSectionName) == 0) {
    if (strcmp(subCategory, lcdSectionName) == 0) {
      return eepromManager.getLcdBrightness();
    } else if (strcmp(subCategory, matrixSectionName) == 0) {
      return eepromManager.getMatrixBrightness();
    }
  } else if (strcmp(mainCategory, contrastSectionName) == 0) {
    if (strcmp(subCategory, lcdSectionName) == 0) {
      return eepromManager.getLcdContrast();
    }
  }
  return -1;
}

void Menu::handleJoystickMenuPress() {
  if (!joystickManager.isPressed() || millis() - lastMenuPress < 400) {
    return;
  }
  lastMenuPress = millis();

  MenuItem* selectedMenuItem = getNthSubOption(currMenuBombRow);
  if (strcmp(selectedMenuItem->label, startSectionName) == 0) {
    canStartGame = true;
    return;
  }
  if (strcmp(selectedMenuItem->label, backSectionName) == 0) {
    currentSubMenu = currentSubMenu->parent;
    resetMenuOptions();
    return;
  }
  if (selectedMenuItem->firstChild != NULL) {
    menuBack.parent = currentSubMenu->parent;
    currentSubMenu = selectedMenuItem;
    resetMenuOptions();
  }
}

void Menu::resetMenuOptions() {
  displayMenuOptions = true;
  currMenuBombRow = 0;
}

void Menu::handleJoystickMenuChange() {
  int oldRow = currMenuBombRow;

  switch (joystickManager.getMovementDirection()) {
    case NONE:
      return;
    case UP:
      if (currMenuBombRow > 0) {
        currMenuBombRow -= 1;
      }
      break;
    case DOWN:
      if (currMenuBombRow < getCurrentSubMenuSize() - 1) {
        currMenuBombRow += 1;
      }
      break;
    case LEFT:
      handleBrightnessAndContrastUpdates(-1);
      handleScrollText(-1);
      break;
    case RIGHT:
      handleBrightnessAndContrastUpdates(1);
      handleScrollText(1);
      break;
    default:
      return;
  }
  // delete old bomb
  if (currMenuBombRow != oldRow) {
    lcd.setCursor(0, oldRow);
    lcd.print(" ");
  }
  displayBomb = true;
  displayMenuOptions = true;
}

void Menu::handleScrollText(int valueMultiplier) {
  if (strcmp(currentSubMenu->label, aboutSectionName) != 0) {
    return;
  }
  MenuItem* currentSubOption = getNthSubOption(currMenuBombRow);
  if (strcmp(currentSubOption->label, githubLink) == 0) {
    updateMenuScrollTextIndex(aboutSectionGitLinkStartIndex, valueMultiplier, githubLink);
  } 
  if (strcmp(currentSubOption->label, creatorName) == 0) {
    updateMenuScrollTextIndex(aboutSectionNameStartIndex, valueMultiplier, creatorName);
  }
}

void Menu::updateMenuScrollTextIndex(int& index, int valueMultiplier, char* sectionName) {
  int newValue = index + valueMultiplier * scrollMenuStep;
  index = (newValue >= 0 && newValue <= strlen(sectionName) - lcdCols + scrollMenuStep) ? newValue : index; 
}

void Menu::handleBrightnessAndContrastUpdates(int valueMultiplier) {
  MenuItem* currentSubOption = getNthSubOption(currMenuBombRow);
  if (currentSubMenu->label != brightnessSectionName && currentSubMenu->label != contrastSectionName) {
    return;
  }

  if (strcmp(currentSubMenu->label, brightnessSectionName) == 0) {
    if (strcmp(currentSubOption->label, lcdSectionName) == 0) {
      eepromManager.updateSettingsValue(lcdBrightnessUpdateStep * valueMultiplier, minLcdBrightness, maxLcdBrightness, eepromManager.lcdBrightnessIndex);
    } else if (strcmp(currentSubOption->label, matrixSectionName) == 0) {
      eepromManager.updateSettingsValue(matrixBrightnessUpdateStep * valueMultiplier, minMatrixBrightness, maxMatrixBrightness, eepromManager.matrixBrightnessIndex);
    }
  } else if (strcmp(currentSubMenu->label, contrastSectionName) == 0) {
    eepromManager.updateSettingsValue(lcdContrastUpdateStep * valueMultiplier, minLcdContrast, maxLcdContrast, eepromManager.lcdContrastIndex);
  }

}

void Menu::displayGameInfo(int bombsUsed, unsigned long elapsedTime) {
  if (oldBombsUsed == bombsUsed && oldElapsedTime == elapsedTime) {
    return;
  }

  oldBombsUsed = bombsUsed;
  oldElapsedTime = elapsedTime;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(gameInfoTexts[0]);
  lcd.print(bombsUsed);
  lcd.write(bombCharIndex);
  lcd.setCursor(0, 1);
  lcd.print(gameInfoTexts[1]);
  lcd.print(elapsedTime);
}

void Menu::displayEndGameMessage(const char* text) {
  if (millis() - lastEndGameLetterChange > endGameMessageUpdateRate) {
    lcd.clear();
    lcd.print(text);
    lcd.setCursor(0, 1);
    displayTextOnLCD(endGameContinueText, lastEndGameLetterIdx, 0, 1);
    lastEndGameLetterChange = millis();
    int textNotOnScreenSize = strlen(endGameContinueText) - lcdCols + 2;
    lastEndGameLetterIdx = (lastEndGameLetterIdx + 1) % textNotOnScreenSize;
  }
}
