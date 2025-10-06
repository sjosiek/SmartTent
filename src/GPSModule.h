#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#include <Arduino.h>
#include <NMEAGPS.h>

class GPSModule {
public:
  // Konstruktor
  GPSModule(Stream &gpsStream);

  // Metody publiczne
  void begin();
  bool update();

  // Gettery do danych
  bool isDataValid() const;
  float getLatitude() const;
  float getLongitude() const;
  float getAltitude() const;
  float getSpeedKph() const;
  float getSpeedKts() const;
  float getHeading() const;
  uint8_t getSatellites() const;

private:
  // Zmienne prywatne
  Stream &_gpsStream;
  NMEAGPS _gps;
  gps_fix _fix;
  
  bool _isValid;
  float _latitude;
  float _longitude;
  float _altitude;
  float _speed_kph;
  float _speed_kts;
  float _heading;
  uint8_t _satellites;
};

#endif