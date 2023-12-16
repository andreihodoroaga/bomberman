#include "Joystick.h"

Joystick::Joystick(int xPin, int yPin, int switchPin, bool commonAnode)
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

void Joystick::handleInput() {
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

Direction Joystick::getMovementDirection() {
  return movementDirection;
}

bool Joystick::isPressed() {
  return pressed;
}

bool Joystick::isLongPressed() {
  return longPressed;
}

void Joystick::handleDirectionChange() {
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

void Joystick::handlePress() {
  switchState = digitalRead(switchPin) == LOW ? HIGH : LOW;

  if (switchState != previousSwitchState) {
    lastSwitchStateUpdate = millis();
  }
  previousSwitchState = switchState;

  if (millis() - lastSwitchStateUpdate > switchPressDebounceDelay) {
    pressed = switchState;
  }

  longPressed = false;
  if (switchState == HIGH && millis() - lastSwitchStateUpdate > longPressTimeThreshold) {
    if (millis() - lastLongPressUpdate < longPressTimeThreshold) {
      return;
    }
    longPressed = true;
    lastLongPressUpdate = millis();
  }
}
