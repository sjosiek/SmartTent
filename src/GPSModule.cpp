#include "GPSModule.h"

GPSModule::GPSModule(Stream &gpsStream) : _gpsStream(gpsStream) {
  // Zerowanie wartości początkowych
  _isValid = false;
  _latitude = 0.0;
  _longitude = 0.0;
  _altitude = 0.0;
  _speed_kph = 0.0;
  _speed_kts = 0.0;
  _heading = 0.0;
  _satellites = 0;
  _isDateTimeValid = false;
  _year = 0;
  _month = 0;
  _day = 0;
  _hour = 0;
  _minute = 0;
  _second = 0;
}

void GPSModule::begin() {
  // Pusta metoda, port szeregowy jest inicjowany w głównym pliku
}

bool GPSModule::update() {
  // Przetwarzamy tylko jeden znak na wywołanie, jeśli jest dostępny.
  // Główna pętla loop() zapewni, że ta funkcja będzie wywoływana wystarczająco często.
  // Serial.println(F("GPSModule::update()"));
  if (!_gpsStream.available()) {
    return false;
  }

  // --- POCZĄTEK SUROWEGO DEBUGGERA ---
  // Odczytujemy jeden znak z portu szeregowego
  char c = _gpsStream.read();
  // I natychmiast wypisujemy go na główny port szeregowy (do monitora)
  // Serial.println(c);
  // --- KONIEC SUROWEGO DEBUGGERA ---
  // Jeśli odebrano i pomyślnie zdekodowano kompletne zdanie NMEA...
  if (_gps.encode(c)) {
    // --- POCZĄTEK BLOKU DEBUGOWANIA ---
    Serial.println(F("\n[GPS DEBUG] Zdekodowano nowe zdanie NMEA."));
    Serial.print(F("  Lokalizacja: "));
    if (_gps.location.isValid()) {
      Serial.print(F("TAK, Lat: ")); Serial.print(_gps.location.lat(), 6);
      Serial.print(F(", Lon: ")); Serial.println(_gps.location.lng(), 6);
    } else {
      Serial.println(F("NIE"));
    }
    Serial.print(F("  Satelity: ")); Serial.println(_gps.satellites.isValid() ? String(_gps.satellites.value()) : F("brak"));
    Serial.print(F("  Data/Czas: "));
    if (_gps.date.isValid() && _gps.time.isValid()) {
      char dt_buf[25];
      snprintf(dt_buf, sizeof(dt_buf), "TAK, %04d-%02d-%02d %02d:%02d:%02d", _gps.date.year(), _gps.date.month(), _gps.date.day(), _gps.time.hour(), _gps.time.minute(), _gps.time.second());
      Serial.println(dt_buf);
    } else {
      Serial.println(F("NIE"));
    }
    // ...aktualizujemy wewnętrzne zmienne klasy.
    _isValid = _gps.location.isValid();
    _isDateTimeValid = _gps.date.isValid() && _gps.time.isValid();

    if (_isValid) {
      _latitude = _gps.location.lat();
      _longitude = _gps.location.lng();
    } else {
      _latitude = 0.0;
      _longitude = 0.0;
    }

    if (_isDateTimeValid) {
      _year = _gps.date.year();
      _month = _gps.date.month();
      _day = _gps.date.day();
      _hour = _gps.time.hour();
      _minute = _gps.time.minute();
      _second = _gps.time.second();
    } else {
      _year = 0; _month = 0; _day = 0;
      _hour = 0; _minute = 0; _second = 0;
    }

    _altitude = _gps.altitude.isValid() ? _gps.altitude.meters() : 0.0;
    _speed_kph = _gps.speed.isValid() ? _gps.speed.kmph() : 0.0;
    _speed_kts = _gps.speed.isValid() ? _gps.speed.knots() : 0.0;
    _heading = _gps.course.isValid() ? _gps.course.deg() : 0.0;
    _satellites = _gps.satellites.isValid() ? _gps.satellites.value() : 0;

    return true; // Zwracamy true, bo dane zostały odświeżone.
  }

  // Jeśli nie odebrano kompletnego zdania, zwracamy false.
  return false;
}

// Implementacja getterów
bool GPSModule::isDataValid() const { return _isValid; }
float GPSModule::getLatitude() const { return _latitude; }
float GPSModule::getLongitude() const { return _longitude; }
float GPSModule::getAltitude() const { return _altitude; }
float GPSModule::getSpeedKph() const { return _speed_kph; }
float GPSModule::getSpeedKts() const { return _speed_kts; }
float GPSModule::getHeading() const { return _heading; }
uint8_t GPSModule::getSatellites() const { return _satellites; }

bool GPSModule::isDateTimeValid() const { return _isDateTimeValid; }
uint16_t GPSModule::getYear() const { return _year; }
uint8_t GPSModule::getMonth() const { return _month; }
uint8_t GPSModule::getDay() const { return _day; }
uint8_t GPSModule::getHour() const { return _hour; }
uint8_t GPSModule::getMinute() const { return _minute; }
uint8_t GPSModule::getSecond() const { return _second; }