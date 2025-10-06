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
}

void GPSModule::begin() {
  // Pusta metoda, port szeregowy jest inicjowany w głównym pliku
}

bool GPSModule::update() {
  if (_gps.available(_gpsStream)) {
    _fix = _gps.read();
    
    _isValid = _fix.valid.location;
    
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