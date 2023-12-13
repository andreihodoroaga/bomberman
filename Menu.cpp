#include "Arduino.h"
#include "Menu.h"

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
    lcd.setCursor(currMenuBombCol, currMenuBombRow % 2);
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
  if (currMenuOption == 0 && currMenuSubOption == -1) {
    // which 2 rows to print (first 2 / last 2)
    int menuRowStart = currMenuBombRow < 2 ? 0 : 2;
    lcd.setCursor(1, 0);
    lcd.print(menuOptions[menuRowStart]);
    lcd.setCursor(1, 1);
    lcd.print(menuOptions[menuRowStart + 1]);
  }
  if (currMenuOption == 1) {
    lcd.noAutoscroll();
    if (currMenuBombRow < 2) {
      displayTextOnLCD(menuAboutSection[0], 0, 1, 0);
      displayTextOnLCD(menuAboutSection[1], aboutSectionNameStartIndex, 1, 1);
    } else {
      displayTextOnLCD(menuAboutSection[2], aboutSectionGitLinkStartIndex, 1, 0);
      displayTextOnLCD(menuAboutSection[3], 0, 1, 1);
    }
  }
  if (currMenuOption == 2) {
    if (currMenuSubOption == -1) {
      displayTextOnLCD(menuSettingsSection[0], 0, 1, 0);
      displayTextOnLCD(menuSettingsSection[1], 0, 1, 1);
    }
    if (currMenuSubOption == 0) {
      if (currMenuBombRow < 2) {
        displayTextOnLCD(brightnessSettings[0], 0, 1, 0);
        displayTextOnLCD(brightnessSettings[1], 0, 1, 1);
        addBrightnessValueOnDisplay();
      } else {
        displayTextOnLCD(brightnessSettings[2], 0, 1, 0);
      }
    }
  }
}

void Menu::addBrightnessValueOnDisplay() {
  lcd.setCursor(brightnessNumberColIdx, 0);
  lcd.print(eepromManager.getLcdBrightness());
  lcd.setCursor(brightnessNumberColIdx, 1);
  lcd.print(eepromManager.getMatrixBrightness());
}

void Menu::handleJoystickMenuPress() {
  if (!joystickManager.isPressed() || millis() - lastMenuPress < 400) {
    return;
  }

  lastMenuPress = millis();

  if (currMenuOption == 0) {
    if (currMenuBombCol == 0 && currMenuBombRow == 0) {
      canStartGame = true;
    }
    if (currMenuBombCol == 0 && currMenuBombRow == 1) {
      currMenuOption = 1;  // about
      resetMenuOptions();
    }
    if (currMenuBombCol == 0 && currMenuBombRow == 2) {
      currMenuOption = 2;  // settings
      resetMenuOptions();
    }
  }

  // handle back
  if ((currMenuOption == 1 && currMenuBombRow == 3) || (currMenuOption == 2 && currMenuBombRow == 1 && currMenuSubOption == -1)) {
    currMenuOption = 0;
    currMenuSubOption = -1;
    canEnterBrightness = false;
    resetMenuOptions();
  }

  if (currMenuOption == 2) {
    if (currMenuSubOption == -1) {
      if (currMenuBombRow == 0 && canEnterBrightness) {
        currMenuSubOption = 0;
        resetMenuOptions();
      } else {
        canEnterBrightness = true;
      }
    } else if (currMenuSubOption == 0) {
      if (currMenuBombRow == 2) {  // back
        currMenuOption = 2;
        currMenuSubOption = -1;
        canEnterBrightness = true;
        resetMenuOptions();
      }
    }
  }
}

void Menu::resetMenuOptions() {
  displayMenuOptions = true;
  currMenuBombRow = 0;
  currMenuBombCol = 0;
}

void Menu::handleJoystickMenuChange() {
  int oldRow = currMenuBombRow;
  int oldCol = currMenuBombCol;
  switch (joystickManager.getMovementDirection()) {
    case UP:
      if (currMenuBombRow > 0) {
        currMenuBombRow -= 1;
      }
      break;
    case DOWN:
      if (currMenuBombRow < currMenuHeight - 1) {
        currMenuBombRow += 1;
      }
      break;
    case LEFT:
      // scroll texts at about section
      if (currMenuOption == 1) {
        if (currMenuBombRow == 1) {
          aboutSectionNameStartIndex = aboutSectionNameStartIndex == 5 ? aboutSectionNameStartIndex - 5 : aboutSectionNameStartIndex;
        } else if (currMenuBombRow == 2) {
          aboutSectionGitLinkStartIndex = aboutSectionGitLinkStartIndex > 0 ? aboutSectionGitLinkStartIndex - 5 : aboutSectionGitLinkStartIndex;
        }
      }
      handleJoystickBrightnessUpdates(-1);
      break;
    case RIGHT:
      if (currMenuOption == 1) {
        if (currMenuBombRow == 1) {
          aboutSectionNameStartIndex = aboutSectionNameStartIndex == 0 ? aboutSectionNameStartIndex + 5 : aboutSectionNameStartIndex;
        } else if (currMenuBombRow == 2) {
          aboutSectionGitLinkStartIndex = aboutSectionGitLinkStartIndex <= 40 ? aboutSectionGitLinkStartIndex + 5 : aboutSectionGitLinkStartIndex;
        }
      }
      handleJoystickBrightnessUpdates(1);
      break;
    default:
      return;
  }
  // delete old bomb
  if (currMenuBombRow != oldRow || currMenuBombCol != oldCol) {
    lcd.setCursor(oldCol, oldRow);
    lcd.print(" ");
  }
  displayBomb = true;
  displayMenuOptions = true;
}

void Menu::handleJoystickBrightnessUpdates(int valueMultiplier) {
  if (currMenuOption == 2 && currMenuSubOption == 0) {
    if (currMenuBombRow == 0) {
      eepromManager.updateLCDBrightness(lcdUpdateStep * valueMultiplier, minLcdBrightness, maxLcdBrightness);
    }
    if (currMenuBombRow == 1) {
      eepromManager.updateMatrixBrightness(matrixUpdateStep * valueMultiplier, minMatrixBrightness, maxMatrixBrightness);
    }
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
