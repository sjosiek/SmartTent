// Plik: SDCard.cpp

#include "SDCard.h"

SDCard::SDCard(uint8_t csPin)
    : _csPin(csPin), _isInitialized(false) {}

bool SDCard::init() {
  _isInitialized = SD.begin(_csPin);
  if (_isInitialized) {
    Serial.println("Karta SD zainicjalizowana pomyślnie.");
  } else {
    Serial.println("Błąd inicjalizacji karty SD!");
  }
  return _isInitialized;
}

void SDCard::logSensorData(const SensorData& data, const char* filename) {
  if (!_isInitialized) {
    return;
  }

  // Sprawdź, czy plik logu istnieje. Jeśli nie, utwórz go i dodaj nagłówek CSV.
  if (!SD.exists(filename)) {
      File dataFile = SD.open(filename, FILE_WRITE);
      if (dataFile) {
        dataFile.println("Date,Time,Temp_BME,Hum_BME,Pressure,Temp_RTC,Temp_DHT,Hum_DHT");
        dataFile.close();
        Serial.println("Utworzono nowy plik logu z nagłówkiem CSV.");
      }
  }

  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    // Formatowanie danych do jednego wiersza CSV z precyzją do 2 miejsc po przecinku
    String dataString = data.dateStr + "," + data.timeForLcd + "," +
                        String(data.temp_bme, 2) + "," + String(data.hum_bme, 2) + "," +
                        String(data.pressure_bme, 2) + "," + String(data.temp_rtc, 2) + "," +
                        String(data.temp_dht, 2) + "," + String(data.hum_dht, 2);

    dataFile.println(dataString);
    dataFile.close(); // Bardzo ważne, aby zamknąć plik i zapisać dane!
  } else {
    Serial.println("Błąd otwarcia pliku logu do zapisu.");
  }
}