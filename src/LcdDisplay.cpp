// Plik: LcdDisplay.cpp

#include "LcdDisplay.h"

LcdDisplay::LcdDisplay(uint8_t address, uint8_t cols, uint8_t rows) : lcd(address, cols, rows) {}

void LcdDisplay::init() {
  lcd.init();
  lcd.backlight();
}

void LcdDisplay::printWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("      SmartTent");
  lcd.setCursor(0, 2);
  lcd.print("    System online");
}

void LcdDisplay::update(String date, String time, float temp_ext, float temp_int, float humidity, float pressure) {
  lcd.setCursor(0, 0);
  lcd.print(date);
  lcd.setCursor(11, 0);
  lcd.print(time);

  lcd.setCursor(0, 1);
  lcd.print("Zewn: ");
  lcd.print(temp_ext, 1);
  lcd.print((char)223);
  lcd.print("C   ");

  lcd.setCursor(0, 2);
  lcd.print("Wewn: ");
  lcd.print(temp_int, 1);
  lcd.print((char)223);
  lcd.print("C   ");

  lcd.setCursor(0, 3);
  lcd.print("Wilg: ");
  lcd.print(humidity, 0);
  lcd.print("% ");
  lcd.print("Cisn: ");
  lcd.print((int)pressure);
  lcd.print("hPa");
}

void LcdDisplay::clear() {
  lcd.clear();
}

void LcdDisplay::noBacklight() {
  lcd.noBacklight();
}

void LcdDisplay::backlight() {
  lcd.backlight();
}


void LcdDisplay::printSleepMessage() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("      Dobranoc!");
}



