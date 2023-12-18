#include "Buzzer.h"

// Use this inside the loop
void Buzzer::play() {
  isMuted = !storage.getIntValueAtIndex(storage.soundsSettingIndex);

  if (!isPlaying || isMuted) {
    return;
  }
  if (millis() - lastBuzzerPlayTime < duration) {
    tone(pin, frequency);
  } else {
    noTone(pin);
    isPlaying = false;
  }
}

void Buzzer::startPlaying(int newDuration, int newFrequency) {
  if (isMuted) {
    return;
  }
  duration = newDuration;
  frequency = newFrequency;
  lastBuzzerPlayTime = millis();
  isPlaying = true;
}
