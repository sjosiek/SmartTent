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
  void printStatus(const char* module, const char* status, int row); // NOWA METODA
  void showTemporaryMessage(const char* line1, const char* line2, uint32_t duration);
  // NOWE METODY: Umożliwiają ustawienie kursora i wypisanie tekstu
  void nextScreen();
  void previousScreen();
  void setCursor(uint8_t col, uint8_t row);
  void print(const char* text);
  void clear();
  void noBacklight();
  void backlight();

private:
  // ZMIANA: Enum do zarządzania ekranami
  enum class LcdScreen {
    MAIN,
    TRACKER,
    GPS,
    // Dodaj tu kolejne ekrany w przyszłości
    SCREEN_COUNT // Specjalny element do zliczania
  };

  bool checkAndInit(); // Prywatna metoda pomocnicza
  // ZMIANA: Prywatne metody do rysowania poszczególnych ekranów
  void _drawMainScreen(const SensorData& data);
  void _drawTrackerScreen(const SensorData& data);
  void _drawGpsScreen(const SensorData& data);
  LiquidCrystal_I2C lcd;
  uint8_t _address;
  bool _isInitialized;
  // ZMIANA: Zmienne do obsługi wiadomości tymczasowych
  uint32_t _tempMessageEndTime = 0;
  bool _isTempMessageActive = false;
  LcdScreen _currentScreen = LcdScreen::MAIN;
};

#endif