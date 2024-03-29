#include "Arduino.h"
#include "Menu.h"

Menu::Menu(LiquidCrystal& lcdObj, Joystick& joystickObj, Storage& storageObj, Buzzer& buzzerObj)
  : lcd(lcdObj), joystick(joystickObj), storage(storageObj), buzzer(buzzerObj) {
  mainMenu.addFirstChild(&menuStart);
  menuHighScore.addFirstChild(&highScoreOne);
  menuSettings.addFirstChild(&settingsName);
  menuAbout.addFirstChild(&aboutGameName);
  settingsName.addFirstChild(&namePlayer);
  soundsSetting.label = getSoundsSetting();
  settingsSounds.addFirstChild(&soundsSetting);
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
  showResetScoresIcon();
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
    char buffer[storage.howToPlaySize];
    storage.getHowToPlayMessage(buffer);
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
  if (currentSubMenu->label != brightnessSectionName &&
      currentSubMenu->label != contrastSectionName &&
      currentSubMenu->label != nameSection &&
      currentSubMenu->label != highScoreSection) {
    return;
  }

  int writeIdx = strlen(subOption->label) + 2;  // +2 because of the bomb at index 0
  lcd.setCursor(writeIdx, row % 2);
  char settingValue[maxSettingsValueLength];
  getSettingStorageValue(currentSubMenu->label, subOption->label, settingValue);
  if (settingValue == NULL) {
    return;
  }
  lcd.print(settingValue);
}

void Menu::getSettingStorageValue(char* mainCategory, char* subCategory, char* buffer) {
  if (strcmp(mainCategory, brightnessSectionName) == 0 && currMenuBombRow < 2) {
    if (strcmp(subCategory, lcdSectionName) == 0) {
      const int displayedBrightness = getDisplayedNumericalSetting(storage.getLcdBrightness(), minLcdBrightness, lcdBrightnessUpdateStep);
      itoa(displayedBrightness, buffer, 10);
    } else if (strcmp(subCategory, matrixSectionName) == 0) {
      const int displayedMatrixBrightness = getDisplayedNumericalSetting(storage.getMatrixBrightness(), minMatrixBrightness, matrixBrightnessUpdateStep);
      itoa(displayedMatrixBrightness, buffer, 10);
    }
  } else if (strcmp(mainCategory, contrastSectionName) == 0 && strcmp(subCategory, lcdSectionName) == 0) { 
      const int displayedLcdContrast = getDisplayedNumericalSetting(storage.getLcdContrast(), minLcdContrast, lcdContrastUpdateStep);
    itoa(displayedLcdContrast, buffer, 10);
  } else if (strcmp(mainCategory, nameSection) == 0 && strcmp(subCategory, playerNameSection) == 0) {
    storage.getPlayerName(buffer);
  } else if (strcmp(mainCategory, highScoreSection) == 0) {
    if (strcmp(subCategory, highScoreLabels[0]) == 0) {
      getHighScoreForPlayer(buffer, 0);
    } else if (strcmp(subCategory, highScoreLabels[1]) == 0) {
      getHighScoreForPlayer(buffer, 1);
    } else if (strcmp(subCategory, highScoreLabels[2]) == 0) {
      getHighScoreForPlayer(buffer, 2);
    } else {
      *buffer = NULL;
    }
  } else {
    *buffer = NULL;
  }
}

const int Menu::getDisplayedNumericalSetting(int value, int minValue, int step) {
  return (value - minValue) / step;
}

void Menu::getHighScoreForPlayer(char* buffer, int index) {
  char name[storage.playerNameSize];
  char score[maxScoreDigits];
  storage.getHighScorePlayerName(name, index);
  itoa(storage.getHighScore(index), score, 10);
  strcpy(buffer, name);
  buffer[storage.playerNameSize - 1] = ' ';
  strcpy(buffer + storage.playerNameSize, score);
}

void Menu::handleJoystickMenuPress() {
  if (!joystick.isPressed() || millis() - lastMenuPress < menuPressDelay) {
    return;
  }
  lastMenuPress = millis();
  buzzer.startPlaying(Buzzer::menuPressDuration, Buzzer::menuPressFrequency);

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
  if (strcmp(selectedMenuItem->label, settingsResetHighScores.label) == 0) {
    resetHighScores();
    showResetIcon = true;
    resetScoresIconShowTime = millis();
  }
  if (strcmp(currentSubMenu->label, nameSection) == 0 && strcmp(selectedMenuItem->label, playerNameSection) == 0) {
    isEditingName = !isEditingName;
    displayMenuOptions = true;
  }
  if (strcmp(currentSubMenu->label, soundSectionLabel) == 0 && strcmp(selectedMenuItem->label, soundsSetting.label) == 0) {
    byte soundOn = storage.getByteValueAtIndex(storage.soundsSettingIndex);
    storage.updateValueAtIndex(storage.soundsSettingIndex, !soundOn);
    soundsSetting.label = getSoundsSetting();
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

  switch (joystick.getMovementDirection()) {
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
    char playerName[storage.playerNameSize];
    getSettingStorageValue(nameSection, playerNameSection, playerName);
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
  char currChar = storage.getPlayerNameCharacter(currentNameIdx);
  char newValue = (currChar + direction >= 'a' && currChar + direction <= 'z') ? currChar + direction : currChar;
  storage.updatePlayerNameCharacter(currentNameIdx, newValue);
}

void Menu::handlePlayerNameEdit(int direction) {
  if (isEditingName) {
    int newValue = currentNameIdx + direction;
    currentNameIdx = (newValue >= 0 && newValue < storage.playerNameSize - 1) ? newValue : currentNameIdx;
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
      updateMenuScrollTextIndex(howToPlaySectionStartIndex, valueMultiplier, storage.howToPlaySize);
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
      storage.updateSettingsValue(lcdBrightnessUpdateStep * valueMultiplier, minLcdBrightness, maxLcdBrightness, storage.lcdBrightnessIndex);
    } else if (strcmp(currentSubOption->label, matrixSectionName) == 0) {
      storage.updateSettingsValue(matrixBrightnessUpdateStep * valueMultiplier, minMatrixBrightness, maxMatrixBrightness, storage.matrixBrightnessIndex);
    }
  } else if (strcmp(currentSubMenu->label, contrastSectionName) == 0) {
    storage.updateSettingsValue(lcdContrastUpdateStep * valueMultiplier, minLcdContrast, maxLcdContrast, storage.lcdContrastIndex);
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
  lcd.print(storage.getRoom());
  lcd.setCursor(clockIconDisplayCol, clockIconDisplayRow);
  lcd.write(clockCharIndex);
  lcd.setCursor(clockIconDisplayCol + 1, clockIconDisplayRow);
  lcd.print(elapsedTime);
}

char* Menu::getSoundsSetting() {
  byte soundOn = storage.getByteValueAtIndex(storage.soundsSettingIndex);
  return soundOn ? soundOnLabel : soundOffLabel;
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

void Menu::resetHighScores() {
  for (int i = 0; i < storage.numStoredHighScores; i++) {
    storage.setHighScore(i, storage.defaultHighScoreValue);
    storage.setHighScorePlayerName(' ', i);
  }
}

void Menu::showResetScoresIcon() {
  if (!showResetIcon || !strcmp(currentSubMenu->label, settingsResetHighScores.label)) {
    return;
  }
  lcd.setCursor(resetScoresIconCol, resetScoresIconRow);
  lcd.write(resetHighScoresCharIndex);
  if (millis() - resetScoresIconShowTime > showResetScoresIconTime) {
    showResetIcon = false;
    lcd.setCursor(resetScoresIconCol, resetScoresIconRow);
    lcd.write(' ');
  }
}
