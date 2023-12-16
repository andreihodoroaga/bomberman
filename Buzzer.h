#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"

class Buzzer {
private:
public:
  static const int pin = A4;
  void play(int duration, int frequency);
  bool isPlaying = false;
  unsigned long lastBuzzerPlayTime = 0;
};

#endif