// Plik: LedDisplay.cpp

#include "LedDisplay.h"

LedDisplay::LedDisplay(uint8_t clkPin, uint8_t dioPin) : display(clkPin, dioPin) {
  colonVisible = false;
}

void LedDisplay::init(uint8_t brightness) {
  display.setBrightness(brightness);
}

void LedDisplay::update(uint8_t hour, uint8_t minute) {
  colonVisible = !colonVisible;

  int timeValue = hour * 100 + minute;
  
  uint8_t colonBitmask = colonVisible ? 0b01000000 : 0;
  
  display.showNumberDecEx(timeValue, colonBitmask, true);
}
