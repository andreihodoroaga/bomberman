#include "HardwareSerial.h"
#include "JoystickManager.h"

JoystickManager::JoystickManager(int xPin, int yPin, int switchPin, bool commonAnode)
  : xPin(xPin), yPin(yPin), switchPin(switchPin), commonAnode(commonAnode) {
  xValue = 0;
  yValue = 0;
  previousSwitchState = LOW;
  switchState = LOW;
  lastSwitchStateUpdate = 0;
  lastLongPressUpdate = 0;
  longPressed = false;
  pressed = false;
  movementDirection = NONE;

  pinMode(switchPin, INPUT_PULLUP);
  if (commonAnode) {
    switchState = !switchState;
  }
}

void JoystickManager::handleInput() {
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);
  yValue = maxValue - yValue;

  bool withinThreshold = (xValue >= movementMinThreshold && xValue <= movementMaxThreshold && yValue >= movementMinThreshold && yValue <= movementMaxThreshold);

  if (!withinThreshold) {
    if (millis() - lastSwitchStateUpdate > movementMinThreshold) {
      lastSwitchStateUpdate = millis();
      handleDirectionChange();
    } else {
      movementDirection = NONE;
    }
  } else {
    movementDirection = NONE;
  }

  handlePress();
}

Direction JoystickManager::getMovementDirection() {
  return movementDirection;
}

bool JoystickManager::isPressed() {
  return pressed;
}

bool JoystickManager::isLongPressed() {
  return longPressed;
}

void JoystickManager::handleDirectionChange() {
  if (xValue < movementMinThreshold) {
    movementDirection = LEFT;
  }
  if (xValue > movementMaxThreshold) {
    movementDirection = RIGHT;
  }
  if (yValue < movementMinThreshold) {
    movementDirection = DOWN;
  }
  if (yValue > movementMaxThreshold) {
    movementDirection = UP;
  }
}

void JoystickManager::handlePress() {
  switchState = digitalRead(switchPin) == LOW ? HIGH : LOW;
  
  if (switchState != previousSwitchState) {
    lastSwitchStateUpdate = millis();
  }

  if (millis() - lastSwitchStateUpdate > switchPressDebounceDelay) {
    if (pressed != switchState) {
      pressed = switchState;
    }
  }
  previousSwitchState = switchState; 

  longPressed = false;
  if (switchState == HIGH && millis() - lastSwitchStateUpdate > longPressTimeThreshold) {
    if (millis() - lastLongPressUpdate < longPressTimeThreshold) {
      return;
    }
    longPressed = true;
    lastLongPressUpdate = millis();
  }
}
