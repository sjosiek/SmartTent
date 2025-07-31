// Plik: LcdDisplay.h

#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

class LcdDisplay {
public:
  LcdDisplay(uint8_t address, uint8_t cols, uint8_t rows);
  void init();
  void printWelcomeMessage();
  void printSleepMessage(); // NOWA METODA
  void update(String date, String time, float temp_bme, float hum_bme, float temp_rtc, float press_bme, float temp_dht, float hum_dht);
  void clear();
  void noBacklight();
  void backlight();

private:
  LiquidCrystal_I2C lcd;
};

#endif