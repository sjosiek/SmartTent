// Plik: LcdDisplay.h

#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include "SensorData.h" // Dołączamy nową strukturę

class LcdDisplay {
public:
  LcdDisplay(uint8_t address, uint8_t cols, uint8_t rows);
  void init();
  void printWelcomeMessage();
  void printSleepMessage(); // NOWA METODA
  void update(const SensorData& data);
  void clear();
  void noBacklight();
  void backlight();

private:
  bool checkAndInit(); // Prywatna metoda pomocnicza
  LiquidCrystal_I2C lcd;
  uint8_t _address;
  bool _isInitialized;
};

#endif