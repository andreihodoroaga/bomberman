#include "Buzzer.h"

// Use this inside the loop
void Buzzer::play(int duration, int frequency) {
  if (millis() - lastBuzzerPlayTime < duration) {
    tone(pin, frequency);
  } else {
    noTone(pin);
    isPlaying = false;
  }
}