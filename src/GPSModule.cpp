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
  if (_gps.available(_gpsStream)) {
    _fix = _gps.read();
    
    _isValid = _fix.valid.location;
    _isDateTimeValid = _fix.valid.date && _fix.valid.time;
    
    if (_fix.valid.location) {
      _latitude = _fix.latitude();
      _longitude = _fix.longitude();
    }
    if (_fix.valid.altitude) {
      _altitude = _fix.altitude();
    }
    if (_fix.valid.speed) {
      _speed_kph = _fix.speed_kph();
      _speed_kts = _fix.speed(); // NeoGPS speed() returns knots
    }
    if (_fix.valid.heading) {
      _heading = _fix.heading();
    }
    if (_fix.valid.satellites) {
      _satellites = _fix.satellites;
    }
    if (_fix.valid.date) {
      _year = _fix.dateTime.year;
      _month = _fix.dateTime.month;
      _day = _fix.dateTime.day;
    }
    if (_fix.valid.time) {
      _hour = _fix.dateTime.hours;
      _minute = _fix.dateTime.minutes;
      _second = _fix.dateTime.seconds;
    }
    
    return true;
  }
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