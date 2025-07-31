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

void LcdDisplay::update(String date, String time, float temp_bme, float hum_bme, float temp_rtc, float press_bme, float temp_dht, float hum_dht) {
  char buffer[21]; // Bufor na formatowane linie

  // Linia 0: Data i czas
  lcd.setCursor(1, 0); // POPRAWKA: data powinna być w pierwszej linii (0)
  lcd.print(date);
  lcd.setCursor(12, 0);
  lcd.print(time);

  // Linia 1: Dane z zewnątrz (BME280)
  lcd.setCursor(0, 1);
  snprintf(buffer, sizeof(buffer), "Z: %4.1f%cC  H: %3.0f%%  ", temp_bme, (char)223, hum_bme);
  lcd.print(buffer);

  // Linia 2: Dane wewnętrzne (RTC) i ciśnienie (BME280)
  lcd.setCursor(0, 2);
  snprintf(buffer, sizeof(buffer), "W: %4.1f%cC  P: %4dhPa", temp_rtc, (char)223, (int)press_bme);
  lcd.print(buffer);

  // Linia 3: Dane z namiotu (DHT11)
  lcd.setCursor(0, 3);
  snprintf(buffer, sizeof(buffer), "N: %4.1f%cC  H: %3.0f%%  ", temp_dht, (char)223, hum_dht);
  lcd.print(buffer);
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
