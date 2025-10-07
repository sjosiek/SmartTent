// Plik: HardwareConfigReader.h

#ifndef HARDWARE_CONFIG_READER_H
#define HARDWARE_CONFIG_READER_H

#include <Arduino.h>

class HardwareConfigReader {
public:
  HardwareConfigReader(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin);
  void begin();
  uint16_t readSwitches(); // Zwraca 16 bitów

private:
  uint8_t _latchPin, _clockPin, _dataPin;
};

#endif