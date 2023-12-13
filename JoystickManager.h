#ifndef JOYSTICK_MANAGER_H
#define JOYSTICK_MANAGER_H

#include "Arduino.h"

enum Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  NONE
};

class JoystickManager {
public:
  JoystickManager(int xPin, int yPin, int switchPin, bool commonAnode);

  void handleInput();
  Direction getMovementDirection();
  bool isPressed();
  bool isLongPressed();

private:
  const int longPressTimeThreshold = 1500;
  const int maxValue = 1023;
  const int movementMinThreshold = 350;
  const int movementMaxThreshold = 650;
  const int switchPressDebounceDelay = 10;

  int xPin;
  int yPin;
  int switchPin;
  bool commonAnode;

  int xValue;
  int yValue;
  byte previousSwitchState;
  byte switchState;
  unsigned long lastSwitchStateUpdate;
  unsigned long lastLongPressUpdate;
  bool longPressed;
  bool pressed;

  Direction movementDirection;

  void handleDirectionChange();
  void handlePress();
};

#endif  
