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

  // Nowe gettery do daty i czasu
  bool isDateTimeValid() const;
  uint16_t getYear() const;
  uint8_t getMonth() const;
  uint8_t getDay() const;
  uint8_t getHour() const;
  uint8_t getMinute() const;
  uint8_t getSecond() const;

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

  // Nowe zmienne prywatne do daty i czasu
  bool _isDateTimeValid;
  uint16_t _year;
  uint8_t _month;
  uint8_t _day;
  uint8_t _hour;
  uint8_t _minute;
  uint8_t _second;
};

#endif