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
             data.dateStr.c_str(), data.timeForLcd.c_str(),
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

  Serial.println(F("Odczytuję plik konfiguracyjny..."));
  while (configFile.available()) {
    String line = configFile.readStringUntil('\n');
    line.trim();

    // Ignoruj puste linie i komentarze
    if (line.length() == 0 || line.startsWith("#")) {
      continue;
    }

    int separatorIndex = line.indexOf('=');
    if (separatorIndex != -1) {
      String key = line.substring(0, separatorIndex);
      String value = line.substring(separatorIndex + 1);

      if (key == "active_minutes") config.activeModeMinutes = value.toInt();
      if (key == "sensor_interval") config.sensorUpdateIntervalMs = value.toInt();
      if (key == "tracker_interval") config.trackerUpdateIntervalMs = value.toInt();
      if (key == "led_brightness") config.ledBrightness = value.toInt();
    }
  }
  configFile.close();
  return true;
}

bool SDCard::writeConfiguration(const Configuration& config, const char* filename) {
  if (!_isInitialized || _writeErrorOccurred) {
    return false;
  }

  // Usuwamy stary plik konfiguracyjny, aby zapisać nowy od zera
  if (SD.exists(filename)) {
    SD.remove(filename);
  }

  File configFile = SD.open(filename, FILE_WRITE);
  if (!configFile) {
    Serial.print(F("Nie można utworzyć pliku konfiguracyjnego do zapisu: "));
    Serial.println(filename);
    _writeErrorOccurred = true; // Ustawiamy flagę błędu
    return false;
  }

  // Zapisujemy konfigurację w formacie klucz=wartość
  configFile.println("# Konfiguracja systemu SmartTent (zapisana automatycznie)");
  configFile.print("active_minutes="); configFile.println(config.activeModeMinutes);
  configFile.print("sensor_interval="); configFile.println(config.sensorUpdateIntervalMs);
  configFile.print("tracker_interval="); configFile.println(config.trackerUpdateIntervalMs);
  configFile.print("led_brightness="); configFile.println(config.ledBrightness);

  configFile.close();
  Serial.println(F("Konfiguracja została pomyślnie zapisana na karcie SD."));
  return true;
}
