// Plik: SDCard.cpp

#include "SDCard.h"

SDCard::SDCard(uint8_t csPin)
    : _csPin(csPin), _isInitialized(false), _writeErrorOccurred(false) {}

bool SDCard::init() {
  _isInitialized = SD.begin(_csPin);

  // Resetujemy flagę błędu przy każdej inicjalizacji
  _writeErrorOccurred = false;

  if (_isInitialized) {
    Serial.println(F("Karta SD zainicjalizowana pomyślnie."));
  } else {
    Serial.println(F("Błąd inicjalizacji karty SD!"));
  }
  return _isInitialized;
}

bool SDCard::isOK() const { return !_writeErrorOccurred; }

void SDCard::logSensorData(const SensorData& data, const char* filename) {
  if (!_isInitialized || _writeErrorOccurred) {
    return;
  }

  // Sprawdź, czy plik logu istnieje. Jeśli nie, utwórz go i dodaj nagłówek CSV.
  if (!SD.exists(filename)) {
      File dataFile = SD.open(filename, FILE_WRITE);
      if (dataFile) {
        dataFile.println("Date|Time|Temp_BME|Hum_BME|Pressure|Temp_RTC|Temp_DHT|Hum_DHT");
        dataFile.close();
        Serial.println(F("Utworzono nowy plik logu z nagłówkiem CSV."));
      }
  }

  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    // ZMIANA: Używamy statycznego bufora i snprintf zamiast klasy String,
    // aby uniknąć dynamicznej alokacji pamięci i fragmentacji sterty.
    char buffer[128];
    char floatBuffer[6][10]; // Bufory na 6 wartości float
    // Konwertujemy wszystkie floaty na stringi za pomocą dtostrf
    dtostrf(data.temp_bme, 4, 2, floatBuffer[0]);
    dtostrf(data.hum_bme, 4, 2, floatBuffer[1]);
    dtostrf(data.pressure_bme, 4, 2, floatBuffer[2]);
    dtostrf(data.temp_rtc, 4, 2, floatBuffer[3]);
    dtostrf(data.temp_dht, 4, 2, floatBuffer[4]);
    dtostrf(data.hum_dht, 4, 2, floatBuffer[5]);

    int written = snprintf(buffer, sizeof(buffer), "%s|%s|%s|%s|%s|%s|%s|%s",
             data.dateStr, data.timeForLcd,
             floatBuffer[0], floatBuffer[1], floatBuffer[2], floatBuffer[3], floatBuffer[4], floatBuffer[5]);

    if (written > 0) dataFile.println(buffer);
    dataFile.close();
  } else {
    // Jeśli nie można otworzyć pliku do zapisu, prawdopodobnie karta jest pełna lub uszkodzona.
    _writeErrorOccurred = true;
    Serial.println(F("Błąd otwarcia pliku logu do zapisu."));
  }
}

bool SDCard::readConfiguration(const char* filename, Configuration& config) {
  if (!_isInitialized) {
    return false;
  }

  File configFile = SD.open(filename, FILE_READ);
  if (!configFile) {
    Serial.print(F("Nie można otworzyć pliku konfiguracyjnego: "));
    Serial.println(filename);
    return false;
  }

  Serial.println(F("Odczytuję plik konfiguracyjny (tryb zoptymalizowany)..."));
  char lineBuffer[64]; // Bufor na jedną linię pliku

  while (configFile.available()) {
    int bytesRead = configFile.readBytesUntil('\n', lineBuffer, sizeof(lineBuffer) - 1);
    lineBuffer[bytesRead] = '\0'; // Zakończ string znakiem null

    // Ręczny "trim" - usuń znaki nowej linii/powrotu karetki z końca
    while (bytesRead > 0 && (lineBuffer[bytesRead - 1] == '\r' || lineBuffer[bytesRead - 1] == '\n')) {
      lineBuffer[--bytesRead] = '\0';
    }

    // Ignoruj puste linie i komentarze
    if (bytesRead == 0 || lineBuffer[0] == '#') {
      continue;
    }

    // Użyj strtok do podziału linii na klucz i wartość
    char* key = strtok(lineBuffer, "=");
    char* value = strtok(NULL, "=");

    if (key && value) {
      if (strcmp(key, "active_minutes") == 0) config.activeModeMinutes = atol(value);
      if (strcmp(key, "sensor_interval") == 0) config.sensorUpdateIntervalMs = atol(value);
      if (strcmp(key, "tracker_interval") == 0) config.trackerUpdateIntervalMs = atol(value);
      if (strcmp(key, "led_brightness") == 0) config.ledBrightness = atoi(value);
      if (strcmp(key, "backlight_boot_ms") == 0) config.backlightBootDurationMs = atol(value);
      if (strcmp(key, "backlight_touch_ms") == 0) config.backlightTouchDurationMs = atol(value);
      if (strcmp(key, "backlight_encoder_ms") == 0) config.backlightEncoderDurationMs = atol(value);
      if (strcmp(key, "long_press_ms") == 0) config.longPressThresholdMs = atol(value);
      if (strcmp(key, "gps_no_data_timeout_ms") == 0) config.gpsNoDataTimeoutMs = atol(value);
      if (strcmp(key, "gps_bad_data_timeout_ms") == 0) config.gpsBadDataTimeoutMs = atol(value);
    }
  }
  configFile.close();
  return true;
}

bool SDCard::writeConfiguration(const Configuration& config, const char* filename) {
  if (!_isInitialized) {
    return false;
  }

  // Usuwamy stary plik konfiguracyjny, aby zapisać nowy od zera
  SD.remove(filename);

  File configFile = SD.open(filename, FILE_WRITE);
  if (!configFile) {
    Serial.print(F("Nie można utworzyć pliku konfiguracyjnego do zapisu: "));
    Serial.println(filename);
    _writeErrorOccurred = true;
    return false;
  }

  // ZMIANA: Używamy bufora i snprintf do zapisu, aby uniknąć klasy String
  char buffer[64];
  configFile.println(F("# Konfiguracja systemu SmartTent (zapisana automatycznie)"));
  snprintf(buffer, sizeof(buffer), "active_minutes=%lu", config.activeModeMinutes); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "sensor_interval=%lu", config.sensorUpdateIntervalMs); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "tracker_interval=%lu", config.trackerUpdateIntervalMs); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "led_brightness=%u", config.ledBrightness); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "backlight_boot_ms=%lu", config.backlightBootDurationMs); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "backlight_touch_ms=%lu", config.backlightTouchDurationMs); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "backlight_encoder_ms=%lu", config.backlightEncoderDurationMs); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "long_press_ms=%lu", config.longPressThresholdMs); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "gps_no_data_timeout_ms=%lu", config.gpsNoDataTimeoutMs); configFile.println(buffer);
  snprintf(buffer, sizeof(buffer), "gps_bad_data_timeout_ms=%lu", config.gpsBadDataTimeoutMs); configFile.println(buffer);

  configFile.close();
  Serial.println(F("Konfiguracja została pomyślnie zapisana na karcie SD."));
  return true;
}
