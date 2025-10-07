// Plik: LedDisplay.h

#ifndef LEDDISPLAY_H
#define LEDDISPLAY_H

#include <TM1637Display.h>
#include <Arduino.h>

class LedDisplay {
public:
  LedDisplay(uint8_t clkPin, uint8_t dioPin);
  void init(uint8_t brightness = 7);
  void update(uint8_t hour, uint8_t minute);

private:
  TM1637Display display;
  bool colonVisible;
};

#endif