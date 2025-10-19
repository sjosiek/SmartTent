// Plik: LcdDisplay.h

#ifndef LCDDISPLAY_H
#define LCDDISPLAY_H

#include <LiquidCrystal_I2C.h>
#include <Arduino.h>
#include "SensorData.h" // Dołączamy nową strukturę
#include "DeviceStatus.h" // ZMIANA: Dołączamy nową definicję statusu
#include "Timer.h"        // ZMIANA: Dołączamy Timer do automatycznej paginacji

class LcdDisplay {
public:
  LcdDisplay(uint8_t address, uint8_t cols, uint8_t rows);
  void init();
  void printWelcomeMessage();
  void printSleepMessage(); // NOWA METODA
  void update(const SensorData& data);
  void printStatus(const char* module, const char* status, int row); // NOWA METODA
  void printLine(const char* text, int row);
  void showTemporaryMessage(const char* line1, const char* line2, uint32_t duration);
  // NOWE METODY: Umożliwiają ustawienie kursora i wypisanie tekstu
  void nextScreen();
  void showMainScreen(); // NOWA METODA: Bezpośrednie przejście do ekranu głównego
  void previousScreen();
  void setCursor(uint8_t col, uint8_t row);    // PRZYWRÓCONA METODA
  void print(const char* text);                // PRZYWRÓCONA METODA
  void print(const __FlashStringHelper* text);
  void clear();
  void noBacklight();
  void backlight();
  // ZMIANA: Metoda do przekazania wskaźnika na tablicę statusów
  void setModuleStatuses(const ModuleStatus* statuses, int count);
  void nextStatusPage(); // NOWA METODA: Ręczne przełączenie strony statusu
  
  // ZMIANA: Enum musi być publiczny, aby można go było używać jako typ zwracany
  enum class LcdScreen {
    MAIN,
    STATUS, // NOWY EKRAN
    TRACKER,
    GPS,
    SCREEN_COUNT // Specjalny element do zliczania
  };
  LcdScreen getCurrentScreen() const; // ZMIANA: Dodajemy getter dla aktualnego ekranu

private:
  bool checkAndInit(); // Prywatna metoda pomocnicza
  // ZMIANA: Prywatne metody do rysowania poszczególnych ekranów
  void _drawMainScreen(const SensorData& data);
  void _drawTrackerScreen(const SensorData& data);
  void _drawGpsScreen(const SensorData& data);
  void _drawStatusScreen(); // NOWA METODA
  LiquidCrystal_I2C lcd;
  uint8_t _address;
  uint8_t _cols;
  uint8_t _rows;
  bool _isInitialized;
  // ZMIANA: Zmienne do obsługi wiadomości tymczasowych
  uint32_t _tempMessageEndTime = 0;
  bool _isTempMessageActive = false;
  LcdScreen _currentScreen = LcdScreen::MAIN;
  // ZMIANA: Wskaźniki do przechowywania informacji o statusach
  const ModuleStatus* _moduleStatuses = nullptr;
  int _moduleStatusCount = 0;
  int _statusScreenPage = 0; // Do paginacji ekranu statusu
  Timer _statusPageTimer;    // ZMIANA: Timer do automatycznego przewijania stron statusu
};

#endif