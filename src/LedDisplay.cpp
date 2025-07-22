// Plik: LedDisplay.cpp

#include "LedDisplay.h"

LedDisplay::LedDisplay(uint8_t clkPin, uint8_t dioPin) : display(clkPin, dioPin) {
  colonVisible = false;
}

void LedDisplay::init(uint8_t brightness) {
  display.setBrightness(brightness);
}

void LedDisplay::update(String time) {
  colonVisible = !colonVisible;

  if (time.length() == 5) {
    int hour = time.substring(0, 2).toInt();
    int minute = time.substring(3, 5).toInt();
    int timeValue = hour * 100 + minute;
    
    uint8_t colonBitmask = colonVisible ? 0b01000000 : 0;
    
    display.showNumberDecEx(timeValue, colonBitmask, true);
  }
}

