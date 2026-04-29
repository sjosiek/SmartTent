#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#include <Arduino.h>
#include <TinyGPS++.h> // ZMIANA: Używamy biblioteki TinyGPS++

enum class GPSHealth {
  NO_MODULE,   // brak bajtów na linii UART przez >no_data_timeout
  BAD_DATA,    // bajty są, ale brak poprawnego NMEA przez >bad_data_timeout
  SEARCHING,   // NMEA OK, brak fix
  FIXED        // fix uzyskany (location.isValid())
};

class GPSModule {
public:
  // Konstruktor
  GPSModule(Stream &gpsStream);

  // Metody publiczne
  void begin();
  bool update();
  GPSHealth getHealth(unsigned long noDataTimeoutMs, unsigned long badDataTimeoutMs) const;

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
  TinyGPSPlus _gps; // ZMIANA: Obiekt biblioteki TinyGPS++
  
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

  // Tracking aktywności linii UART do detekcji health
  unsigned long _firstByteAtMs;     // 0 = nigdy nie było bajta
  unsigned long _lastSentenceAtMs;  // 0 = nigdy nie zdekodowano NMEA
};

#endif