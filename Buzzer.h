#ifndef BUZZER_H
#define BUZZER_H

#include "Arduino.h"
#include "Storage.h"

class Buzzer {
private:
int duration;
int frequency;
Storage storage;
public:
  Buzzer(Storage& storageObj): storage(storageObj) {}
  static const int pin = A4;
  static const int menuPressDuration = 50;
  static const int menuPressFrequency = 1000;
  static const int explodeBombFrequency = 400;
  static const int fastBlinkFrequency = 1000;
  void play();
  void startPlaying(int newDuration, int newFrequency);
  bool isPlaying = false;
  bool isMuted;
  unsigned long lastBuzzerPlayTime = 0;
};

#endif