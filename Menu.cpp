#include "Arduino.h"
#include "Menu.h"

Menu::Menu(LiquidCrystal& lcdObj, JoystickManager& joystickManagerObj, EEPROMManager& eepromManagerObj)
  : lcd(lcdObj), joystickManager(joystickManagerObj), eepromManager(eepromManagerObj) {
  mainMenu.addFirstChild(&menuStart);
  menuHighScore.addFirstChild(&highScoreOne);
  menuSettings.addFirstChild(&settingsName);
  menuAbout.addFirstChild(&aboutGameName);
  settingsName.addFirstChild(&namePlayer);
  settingsSounds.addFirstChild(&soundsBack);
  settingsBrightness.addFirstChild(&brightnessMatrix);
  settingsContrast.addFirstChild(&contrastLCD);
  menuHowToPlay.addFirstChild(&howToPlay);
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
  handlePlayerNameChange();
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

  int currRow = currMenuBombRow % 2 == 0 ? currMenuBombRow : currMenuBombRow - 1;
  int nextRow = currMenuBombRow % 2 == 0 ? currMenuBombRow + 1 : currMenuBombRow;
  MenuItem* currSubOption = getNthSubOption(currRow);
  MenuItem* nextSubOption = getNthSubOption(nextRow);
  // The how to play section is very large so we have to load it from eeprom
  // so that it is only stored in local memory when showing this submenu
  if (currentSubMenu->label == howToPlaySectionName) {
    char buffer[eepromManager.howToPlaySize];
    eepromManager.getHowToPlayMessage(buffer);
    displayTextOnLCD(buffer, getTextStartIndex(currentSubMenu->label), 1, 0);
  } else {
    displayTextOnLCD(currSubOption->label, getTextStartIndex(currSubOption->label), 1, 0);
  }
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
  // the extra check is added to make sure the index is not returned for the "How to play" submenu too
  if (strcmp(label, howToPlaySectionName) == 0 && currentSubMenu->label == howToPlaySectionName) {
    return howToPlaySectionStartIndex;
  }
  return 0;
}

void Menu::displaySettingsValue(int row, MenuItem* subOption) {
  if (currentSubMenu->label != brightnessSectionName && currentSubMenu->label != contrastSectionName && currentSubMenu->label != nameSection) {
    return;
  }

  int writeIdx = strlen(subOption->label) + 2;  // +2 because of the bomb at index 0
  lcd.setCursor(writeIdx, row);
  char settingValue[7];
  getSettingEepromValue(currentSubMenu->label, subOption->label, settingValue);
  if (settingValue == NULL) {
    return;
  }
  lcd.print(settingValue);
}

void Menu::getSettingEepromValue(char* mainCategory, char* subCategory, char* buffer) {
  if (strcmp(mainCategory, brightnessSectionName) == 0) {
    if (strcmp(subCategory, lcdSectionName) == 0) {
      itoa(eepromManager.getLcdBrightness(), buffer, 10);
    } else if (strcmp(subCategory, matrixSectionName) == 0) {
      itoa(eepromManager.getMatrixBrightness(), buffer, 10);
    }
  } else if (strcmp(mainCategory, contrastSectionName) == 0 && strcmp(subCategory, lcdSectionName) == 0) {
    itoa(eepromManager.getLcdContrast(), buffer, 10);
  } else if (strcmp(mainCategory, nameSection) == 0 && strcmp(subCategory, playerNameSection) == 0) {
    eepromManager.getPlayerName(buffer);
  } else {
    *buffer = NULL;
  }
}

void Menu::handleJoystickMenuPress() {
  if (!joystickManager.isPressed() || millis() - lastMenuPress < menuPressDelay) {
    return;
  }
  lastMenuPress = millis();

  MenuItem* selectedMenuItem = getNthSubOption(currMenuBombRow);
  if (strcmp(selectedMenuItem->label, startSectionName) == 0) {
    canStartGame = true;
    return;
  }
  if (strcmp(selectedMenuItem->label, backSectionName) == 0) {
    if (currentSubMenu->label == nameSection) {
      isEditingName = false;
    }
    currentSubMenu = currentSubMenu->parent;
    resetMenuOptions();
    return;
  }
  if (strcmp(currentSubMenu->label, nameSection) == 0 && strcmp(selectedMenuItem->label, playerNameSection) == 0) {
    isEditingName = !isEditingName;
    displayMenuOptions = true;
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
      if (isEditingName) {
        handleNameLetterChange(+1);
        break;
      }
      if (currMenuBombRow > 0) {
        currMenuBombRow -= 1;
      }
      break;
    case DOWN:
      if (isEditingName) {
        handleNameLetterChange(-1);
        break;
      }
      if (currMenuBombRow < getCurrentSubMenuSize() - 1) {
        currMenuBombRow += 1;
      }
      break;
    case LEFT:
      handleBrightnessAndContrastUpdates(-1);
      handleScrollText(-1);
      handlePlayerNameEdit(-1);
      break;
    case RIGHT:
      handleBrightnessAndContrastUpdates(1);
      handleScrollText(1);
      handlePlayerNameEdit(1);
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

void Menu::handlePlayerNameChange() {
  if (!isEditingName) {
    return;
  }

  // blink current letter in name
  if (strcmp(currentSubMenu->label, nameSection) == 0 && currMenuBombRow == 0) {
    char playerName[7];
    getSettingEepromValue(nameSection, playerNameSection, playerName);
    char currLetter = playerName[currentNameIdx];
    int writeIdx = strlen(playerNameSection) + 2;
    lcd.setCursor(writeIdx + currentNameIdx, currMenuBombRow);

    if (millis() - lastNameLetterBlink > 2 * nameLetterBlinkDelay) {
      lastNameLetterBlink = millis();
      lcd.write(currLetter);
    } else if (millis() - lastNameLetterBlink > nameLetterBlinkDelay) {
      lcd.write(' ');
    }
  }
}

void Menu::handleNameLetterChange(int direction) {
  lastNameLetterBlink = millis();
  char currChar = eepromManager.getPlayerNameCharacter(currentNameIdx);
  char newValue = (currChar + direction >= 'a' && currChar + direction <= 'z') ? currChar + direction : currChar; 
  eepromManager.updatePlayerNameCharacter(currentNameIdx, newValue);
}

void Menu::handlePlayerNameEdit(int direction) {
  if (isEditingName) {
    int newValue = currentNameIdx + direction;
    currentNameIdx = (newValue >= 0 && newValue < eepromManager.playerNameSize - 1) ? newValue : currentNameIdx;
  }
}

void Menu::handleScrollText(int valueMultiplier) {
  if (strcmp(currentSubMenu->label, aboutSectionName) != 0 && strcmp(currentSubMenu->label, howToPlaySectionName) != 0) {
    return;
  }
  MenuItem* currentSubOption = getNthSubOption(currMenuBombRow);
  if (strcmp(currentSubOption->label, githubLink) == 0) {
    updateMenuScrollTextIndex(aboutSectionGitLinkStartIndex, valueMultiplier, strlen(githubLink));
  }
  if (strcmp(currentSubOption->label, creatorName) == 0) {
    updateMenuScrollTextIndex(aboutSectionNameStartIndex, valueMultiplier, strlen(creatorName));
  }
  if (strcmp(currentSubMenu->label, howToPlaySectionName) == 0) {
    // checking the bomb row for the submenu because of the long text of the "how to play" description
    if (currMenuBombRow == 0) {
      updateMenuScrollTextIndex(howToPlaySectionStartIndex, valueMultiplier, eepromManager.howToPlaySize);
    }
  }
}

void Menu::updateMenuScrollTextIndex(int& index, int valueMultiplier, int sectionNameLength) {
  int newValue = index + valueMultiplier * scrollMenuStep;
  index = (newValue >= 0 && newValue <= sectionNameLength - lcdCols + scrollMenuStep) ? newValue : index;
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
