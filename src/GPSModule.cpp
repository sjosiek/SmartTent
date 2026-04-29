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
  _firstByteAtMs = 0;
  _lastSentenceAtMs = 0;
}

void GPSModule::begin() {
  // Pusta metoda, port szeregowy jest inicjowany w głównym pliku
}

bool GPSModule::update() {
  // Czytamy WSZYSTKIE dostępne bajty per wywołanie (while), żeby nie przepełnić
  // 64-bajtowego bufora UART przy 9600 baud (~960 znaków/s).
  bool newSentence = false;

  while (_gpsStream.available()) {
    if (_firstByteAtMs == 0) {
      _firstByteAtMs = millis();  // pierwszy bajt — sygnał, że moduł jest podpięty
    }

    char c = _gpsStream.read();

    if (_gps.encode(c)) {
      _lastSentenceAtMs = millis();  // pełne zdanie NMEA odebrane

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

      newSentence = true;
    }
  }

  return newSentence;
}

GPSHealth GPSModule::getHealth(unsigned long noDataTimeoutMs, unsigned long badDataTimeoutMs) const {
  unsigned long now = millis();

  // Brak ani jednego bajta na linii — moduł niepodpięty (po przekroczeniu timeoutu).
  // Przed timeoutem pokazujemy SEARCHING, żeby UI nie błyskał na boot.
  if (_firstByteAtMs == 0) {
    return (now > noDataTimeoutMs) ? GPSHealth::NO_MODULE : GPSHealth::SEARCHING;
  }

  // Bajty są, ale przez bad_data_timeout nie zdekodowano poprawnego NMEA.
  if (_lastSentenceAtMs == 0 || (now - _lastSentenceAtMs) > badDataTimeoutMs) {
    return GPSHealth::BAD_DATA;
  }

  return _isValid ? GPSHealth::FIXED : GPSHealth::SEARCHING;
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