#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "Arduino.h"

enum Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT,
  NONE
};

class Joystick {
public:
  Joystick(int xPin, int yPin, int switchPin, bool commonAnode);

  void handleInput();
  Direction getMovementDirection();
  bool isPressed();
  bool isLongPressed();

private:
  const int longPressTimeThreshold = 1500;
  const int maxValue = 1023;
  const int movementMinThreshold = 350;
  const int movementMaxThreshold = 650;
  const int switchPressDebounceDelay = 5;

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
