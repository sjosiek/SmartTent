// Plik: LcdDisplay.cpp

#include "LcdDisplay.h"

const char DEGREE_SYMBOL = 223; // Definicja symbolu stopnia

LcdDisplay::LcdDisplay(uint8_t address, uint8_t cols, uint8_t rows) 
  : lcd(address, cols, rows), _address(address), _isInitialized(false) {}

void LcdDisplay::init() {
  _isInitialized = checkAndInit();
}

bool LcdDisplay::checkAndInit() {
  // Sprawdzamy, czy urządzenie odpowiada na magistrali I2C
  Wire.beginTransmission(_address);
  if (Wire.endTransmission() == 0) {
    // Urządzenie jest obecne, możemy je bezpiecznie zainicjować.
    lcd.init();
    lcd.backlight();
    Serial.println(F("LCD I2C check OK. Inicjalizacja pomyślna."));
    return true;
  } else {
    // Urządzenie nie odpowiada.
    Serial.println(F("BŁĄD: Nie znaleziono wyświetlacza LCD na adresie I2C."));
    return false;
  }
}

void LcdDisplay::printWelcomeMessage() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("      SmartTent");
  lcd.setCursor(0, 2);
  lcd.print("    System online");
}

// Używamy pragm kompilatora, aby lokalnie wyłączyć ostrzeżenie o formatowaniu.
// To uporczywe ostrzeżenie wynika ze specyfiki kompilacji na platformę AVR.
// Jesteśmy pewni, że kod jest bezpieczny, więc możemy świadomie zignorować to ostrzeżenie
// tylko dla tej jednej funkcji, nie wpływając na resztę projektu.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
void LcdDisplay::update(const SensorData& data) {
  if (!_isInitialized) {
    // Jeśli wyświetlacz nie jest zainicjalizowany, spróbuj go zainicjować.
    _isInitialized = checkAndInit();
    if (!_isInitialized) return;
  }

  // ZMIANA: Sprawdzamy, czy aktywna jest wiadomość tymczasowa.
  if (_isTempMessageActive && millis() < _tempMessageEndTime) {
    return; // Jeśli tak, nie nadpisujemy jej danymi z czujników.
  } else if (_isTempMessageActive) {
    // Jeśli wyświetlacz nie jest zainicjalizowany (np. po utracie zasilania),
    // spróbuj go zainicjalizować ponownie.
    _isInitialized = checkAndInit();
    if (!_isInitialized) {
      // Jeśli inicjalizacja się nie powiodła, nie próbuj pisać, aby uniknąć śmieci.
      return;
    }
    _isTempMessageActive = false; // Czas minął, dezaktywujemy flagę.
  }
  char buffer[21]; // Bufor na formatowane linie

  // Linia 0: Data i czas
  lcd.setCursor(0, 0);
  // Format: "YYYY-MM-DD  HH:MM:SS" (10 + 2 + 8 = 20 znaków)
  snprintf(buffer, sizeof(buffer), "%s  %s", data.dateStr.c_str(), data.timeForLcd.c_str());
  lcd.print(buffer);

  // Linia 1: Dane z namiotu (DHT11)
  lcd.setCursor(0, 1);
  snprintf(buffer, sizeof(buffer), "N: %4.1f%cC  H: %3.0f%%  ", (double)data.temp_dht, DEGREE_SYMBOL, (double)data.hum_dht);
  lcd.print(buffer); 

  // Linia 2: Dane z zewnątrz (BME280)
  lcd.setCursor(0, 2);
  snprintf(buffer, sizeof(buffer), "Z: %4.1f%cC H: %3.0f%%  ", (double)data.temp_bme, DEGREE_SYMBOL, (double)data.hum_bme);
  lcd.print(buffer);

  // Linia 3: Aktualne pozycje serwomechanizmów
  lcd.setCursor(0, 3);
  // Zmieniono formatowanie ciśnienia, aby pokazywało 2 miejsca po przecinku.
  // Format został zacieśniony, aby zmieścić się w 20 kolumnach wyświetlacza.
  // Użyto "%7.2f", aby zapewnić stałą szerokość i wyrównanie.
  snprintf(buffer, sizeof(buffer), "W:%4.1f%cC P:%7.2fhPa", (double)data.temp_rtc, DEGREE_SYMBOL, (double)data.pressure_bme);
  lcd.print(buffer);
  
}
#pragma GCC diagnostic pop

void LcdDisplay::printStatus(const char* module, const char* status, int row) {
  if (!_isInitialized) return;
  lcd.setCursor(0, row);
  char buffer[21]; // 20 kolumn + znak null

  // Formatowanie z wyrównaniem do lewej, aby statusy były w jednej linii
  // np. "Zegar RTC        [OK]"
  snprintf(buffer, sizeof(buffer), "%-16s [%s]", module, status);
  lcd.print(buffer);
}

void LcdDisplay::showTemporaryMessage(const char* line1, const char* line2, uint32_t duration) {
  if (!_isInitialized) return;
  
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(line1);
  lcd.setCursor(0, 2);
  lcd.print(line2);
  
  _tempMessageEndTime = millis() + duration;
  _isTempMessageActive = true;
}

void LcdDisplay::setCursor(uint8_t col, uint8_t row) {
  if (!_isInitialized) return;
  lcd.setCursor(col, row);
}

void LcdDisplay::print(const char* text) {
  if (!_isInitialized) return;
  lcd.print(text);
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
void LcdDisplay::backlight() {
  lcd.backlight();
}


void LcdDisplay::printSleepMessage() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("      Dobranoc!");
}
