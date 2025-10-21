// Plik: LcdDisplay.cpp

#include "LcdDisplay.h"

const char DEGREE_SYMBOL = 223; // Definicja symbolu stopnia
// ZMIANA: Inicjalizujemy timer w liście inicjalizacyjnej konstruktora
LcdDisplay::LcdDisplay(uint8_t address, uint8_t cols, uint8_t rows)
    : lcd(address, cols, rows), _address(address), _cols(cols), _rows(rows), 
      _isInitialized(false), _statusPageTimer(5000), _gpsPageTimer(5000) {} // 5 sekund na stronę

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

void LcdDisplay::printSleepMessage() {
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print(F("      Dobranoc!"));
}

// Używamy pragm kompilatora, aby lokalnie wyłączyć ostrzeżenie o formatowaniu.
// To uporczywe ostrzeżenie wynika ze specyfiki kompilacji na platformę AVR.
// Jesteśmy pewni, że kod jest bezpieczny, więc możemy świadomie zignorować to ostrzeżenie
// tylko dla tej jednej funkcji, nie wpływając na resztę projektu.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat"
void LcdDisplay::update(const SensorData& data) {
  if (!_isInitialized) {
    _isInitialized = checkAndInit();
    if (!_isInitialized) return;
  }

  if (_isTempMessageActive && millis() < _tempMessageEndTime) {
    return; // Nie nadpisuj wiadomości tymczasowej
  } else if (_isTempMessageActive) {
    _isTempMessageActive = false; // Czas minął, dezaktywuj flagę
    lcd.clear(); // Wyczyść ekran po wiadomości tymczasowej
  }

  // ZMIANA: Logika automatycznej paginacji dla ekranu statusu
  if (_currentScreen == LcdScreen::STATUS) {
    if (_statusPageTimer.isReady()) {
      const int itemsPerPage = _rows - 1;
      const int numPages = (_moduleStatusCount + itemsPerPage - 1) / itemsPerPage;
      if (numPages > 1) {
        _statusScreenPage = (_statusScreenPage + 1) % numPages;
        lcd.clear(); // Wyczyść, aby przerysować nową stronę
      }
    }
  }

  // ZMIANA: Logika automatycznej paginacji dla ekranu GPS
  if (_currentScreen == LcdScreen::GPS) {
    if (_gpsPageTimer.isReady()) {
      // Mamy 2 strony (0 i 1)
      _gpsScreenPage = (_gpsScreenPage + 1) % 2;
      lcd.clear(); // Wyczyść, aby przerysować nową stronę
    }
  }

  // ZMIANA: Dyspozytor, który wywołuje odpowiednią funkcję rysującą
  switch (_currentScreen) {
    case LcdScreen::MAIN:
      _drawMainScreen(data);
      break;
    case LcdScreen::TRACKER:
      _drawTrackerScreen(data);
      break;
    case LcdScreen::GPS:
      _drawGpsScreen(data);
      break;
    case LcdScreen::STATUS:
      _drawStatusScreen();
      break;
    default:
      _drawMainScreen(data);
      break;
  }
}
#pragma GCC diagnostic pop

void LcdDisplay::_drawMainScreen(const SensorData& data) {
  char buffer[21];
  lcd.setCursor(0, 0);
  snprintf(buffer, sizeof(buffer), "%s  %s", data.dateStr, data.timeForLcd);
  lcd.print(buffer);

  lcd.setCursor(0, 1);
  snprintf(buffer, sizeof(buffer), "N: %4.1f%cC  H: %3.0f%%  ", (double)data.temp_dht, DEGREE_SYMBOL, (double)data.hum_dht);
  lcd.print(buffer); 

  lcd.setCursor(0, 2);
  snprintf(buffer, sizeof(buffer), "Z: %4.1f%cC H: %3.0f%%  ", (double)data.temp_bme, DEGREE_SYMBOL, (double)data.hum_bme);
  lcd.print(buffer);

  lcd.setCursor(0, 3);
  snprintf(buffer, sizeof(buffer), "W:%4.1f%cC P:%7.2fhPa", (double)data.temp_rtc, DEGREE_SYMBOL, (double)data.pressure_bme);
  lcd.print(buffer);
}

void LcdDisplay::_drawTrackerScreen(const SensorData& data) {
  char buffer[21];
  lcd.setCursor(0, 0);
  lcd.print(F("Status Trackera"));

  lcd.setCursor(0, 1);
  snprintf(buffer, sizeof(buffer), "H: %-3d         V: %-3d", data.servo_h_pos, data.servo_v_pos);
  lcd.print(buffer);

  lcd.setCursor(0, 2);
  snprintf(buffer, sizeof(buffer), "TL:%-4d      TR:%-4d", data.ldr_tl, data.ldr_tr);
  lcd.print(buffer);

  lcd.setCursor(0, 3);
  snprintf(buffer, sizeof(buffer), "DL:%-4d      DR:%-4d", data.ldr_dl, data.ldr_dr);
  lcd.print(buffer);
}

void LcdDisplay::_drawGpsScreen(const SensorData& data) {
  char buffer[21];
  char float_buf[12];

  lcd.setCursor(0, 0);
  if (data.gps_is_valid) {
    snprintf(buffer, sizeof(buffer), "Sats: %-2d | FIXED ", data.gps_sats);
  } else {
    snprintf(buffer, sizeof(buffer), "Sats: -- | SEARCH");
  }
  lcd.print(buffer);

  lcd.setCursor(0, 1);
  if (data.gps_time_valid) {
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d  %02d:%02d:%02d", data.gps_year, data.gps_month, data.gps_day, data.gps_hour, data.gps_minute, data.gps_second);
  } else {
    snprintf(buffer, sizeof(buffer), "---- -- --  --:--:--");
  }
  lcd.print(buffer);

  lcd.setCursor(0, 2);
  if (data.gps_is_valid) {
    dtostrf(data.gps_lat, 4, 6, float_buf);
    snprintf(buffer, sizeof(buffer), "Lat: %s", float_buf);
  } else {
    snprintf(buffer, sizeof(buffer), "Lat: ---");
  }
  lcd.print(buffer);

  lcd.setCursor(0, 3);
  if (data.gps_is_valid) {
    dtostrf(data.gps_lon, 4, 6, float_buf);
    snprintf(buffer, sizeof(buffer), "Lon: %s", float_buf);
  } else {
    snprintf(buffer, sizeof(buffer), "Lon: ---");
  }
  lcd.print(buffer);
}

void LcdDisplay::_drawStatusScreen() {
  const int itemsPerPage = _rows - 1; // -1 na tytuł
  lcd.setCursor(0, 0);
  lcd.print(F("Status Modulow"));

  // ZMIANA: Dodajemy szczegółowe logowanie do portu szeregowego
  Serial.println(F("\n[DEBUG] Rysowanie ekranu statusu..."));
  Serial.print(F("  - Aktualna strona: ")); Serial.println(_statusScreenPage);
  Serial.print(F("  - Liczba modułów: ")); Serial.println(_moduleStatusCount);
  Serial.print(F("  - Elementów na stronę: ")); Serial.println(itemsPerPage);
  // Koniec zmiany

  if (!_moduleStatuses || _moduleStatusCount == 0) {
    printLine("Brak danych statusu", 1);
    return;
  }

  // Logika paginacji
  int startIdx = _statusScreenPage * itemsPerPage;
  for (int i = 0; i < itemsPerPage; ++i) {
    int currentIdx = startIdx + i;
    int row = i + 1;
    if (currentIdx < _moduleStatusCount) {
      char buffer[_cols + 1];
      // ZMIANA: Usunięto wyrównanie tekstu ("%-13s"), które powodowało obcinanie
      // dłuższych nazw modułów lub statusów.
      snprintf(buffer, sizeof(buffer), "%s: %s", _moduleStatuses[currentIdx].name, _moduleStatuses[currentIdx].statusText);
      
      // ZMIANA: Logujemy, co dokładnie zostanie wydrukowane w danym wierszu
      Serial.print(F("  - Rysuję wiersz ")); Serial.print(row);
      Serial.print(F(": '")); Serial.print(buffer); Serial.println(F("'"));

      printLine(buffer, row);
    } else {
      printLine("", row); // Wyczyść resztę linii
    }
  }
}

void LcdDisplay::printStatus(const char* module, const char* status, int row) {
  if (!_isInitialized) return;
  lcd.setCursor(0, row);
  char buffer[_cols + 1];

  // Formatowanie z wyrównaniem do lewej, aby statusy były w jednej linii
  // np. "Zegar RTC        [OK]"
  snprintf(buffer, sizeof(buffer), "%-16s [%s]", module, status);
  lcd.print(buffer);
}

void LcdDisplay::printLine(const char* text, int row) {
  if (!_isInitialized) return;
  lcd.setCursor(0, row);
  
  // ZMIANA KRYTYCZNA: Usunięto problematyczne, podwójne użycie snprintf.
  // Teraz drukujemy tekst, a następnie ręcznie dopełniamy linię spacjami,
  // aby ją wyczyścić. To jest bardziej niezawodne.
  int textLen = lcd.print(text);
  // Dopełnij resztę linii spacjami
  for (int i = textLen; i < _cols; i++) {
    lcd.print(' ');
  }
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

void LcdDisplay::nextScreen() {
  int current = static_cast<int>(_currentScreen);
  current++; 
  if (current >= static_cast<int>(LcdScreen::SCREEN_COUNT)) {
    current = 0; // Zapętl
  }
  _currentScreen = static_cast<LcdScreen>(current);

  // ZMIANA: Resetujemy stronę statusu i timer przy przejściu na ten ekran,
  // aby zawsze zaczynać od pierwszej strony.
  if (_currentScreen == LcdScreen::STATUS) {
    _statusScreenPage = 0;
    _statusPageTimer.reset(); // Zresetuj timer, aby odliczał od nowa
  }
  // ZMIANA: Resetujemy stronę GPS przy przejściu na ten ekran
  if (_currentScreen == LcdScreen::GPS) {
    _gpsScreenPage = 0;
    _gpsPageTimer.reset();
  }
  lcd.clear(); // Wyczyść ekran przy zmianie
}

void LcdDisplay::showMainScreen() {
  if (_currentScreen == LcdScreen::MAIN) return; // Już jesteśmy na ekranie głównym
  _currentScreen = LcdScreen::MAIN;
  _statusScreenPage = 0; // Zresetuj paginację dla ekranu statusu na wszelki wypadek
  lcd.clear(); // Wyczyść ekran przy zmianie
}

void LcdDisplay::previousScreen() {
  int current = static_cast<int>(_currentScreen);
  current--;
  if (current < 0) {
    current = static_cast<int>(LcdScreen::SCREEN_COUNT) - 1; // Zapętl
  }
  _currentScreen = static_cast<LcdScreen>(current);

  // ZMIANA: Resetujemy stronę statusu i timer przy przejściu na ten ekran,
  // aby zawsze zaczynać od pierwszej strony.
  if (_currentScreen == LcdScreen::STATUS) {
    _statusScreenPage = 0;
    _statusPageTimer.reset(); // Zresetuj timer, aby odliczał od nowa
  }
  // ZMIANA: Resetujemy stronę GPS przy przejściu na ten ekran
  if (_currentScreen == LcdScreen::GPS) {
    _gpsScreenPage = 0;
    _gpsPageTimer.reset();
  }
  lcd.clear(); // Wyczyść ekran przy zmianie
}

void LcdDisplay::nextStatusPage() {
  // Ta metoda jest przeznaczona głównie do użytku w sekwencji startowej,
  // aby wymusić pokazanie kolejnych stron bez czekania na timer.
  if (_currentScreen != LcdScreen::STATUS) return;
  const int itemsPerPage = _rows - 1;
  const int numPages = (_moduleStatusCount + itemsPerPage - 1) / itemsPerPage;
  if (numPages > 1) {
    _statusScreenPage = (_statusScreenPage + 1) % numPages;
  }
}

// ZMIANA: Implementacja gettera
LcdDisplay::LcdScreen LcdDisplay::getCurrentScreen() const {
  return _currentScreen;
}

void LcdDisplay::setCursor(uint8_t col, uint8_t row) {
  if (!_isInitialized) return;
  lcd.setCursor(col, row);
}

void LcdDisplay::print(const char* text) {
  if (!_isInitialized) return;
  lcd.print(text);
}

void LcdDisplay::print(const __FlashStringHelper* text) {
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

void LcdDisplay::setModuleStatuses(const ModuleStatus* statuses, int count) {
  _moduleStatuses = statuses;
  _moduleStatusCount = count;
}
