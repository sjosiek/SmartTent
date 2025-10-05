// GPSModule.h

#ifndef GPSMODULE_H
#define GPSMODULE_H

#include <Arduino.h>
#include <SoftwareSerial.h> // Jeśli używasz innego pinu niż RX/TX
#include <TinyGPS++.h>

class GPSModule {
public:
  GPSModule(uint8_t rxPin, uint8_t txPin);
  bool begin();
  void update();

  float getLatitude();
  float getLongitude();
  float getAltitude();
  float getCourse();
  float getSpeedKmph();
  float getSpeedKnots();
  uint8_t getSatellites();

private:
  SoftwareSerial ss;
  TinyGPSPlus gps;

  uint8_t _rxPin;
  uint8_t _txPin;

  float latitude = 0.0;
  float longitude = 0.0;
  float altitude = 0.0;
  uint8_t satellites = 0;

  float course = 0.0;
  float speedKmph = 0.0;
  float speedKnots = 0.0;
};

#endif
