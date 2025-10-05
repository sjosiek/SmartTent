// GPSModule.cpp

#include "GPSModule.h"

GPSModule::GPSModule(uint8_t rxPin, uint8_t txPin) : _rxPin(rxPin), _txPin(txPin), ss(_rxPin, _txPin) {}

bool GPSModule::begin() {
  ss.begin(9600);
  return true;
}

void GPSModule::update() {
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  if (gps.location.isValid()) {
    latitude = gps.location.lat();
    longitude = gps.location.lng();
  } else {
        Serial.print(F("INVALID"));
  }

    if (gps.altitude.isValid()) {
    altitude = gps.altitude.meters();
  } else {
        Serial.print(F("INVALID"));
  }

  if (gps.satellites.isValid()) {
    satellites = gps.satellites.value();
  } else {
        Serial.print(F("INVALID"));
  }
    if (gps.course.isValid()) {
    course = gps.course.deg();
  } else {
        Serial.print(F("INVALID"));
  }

      if (gps.speed.isValid()) {
    speedKmph = gps.speed.kmph();
        speedKnots = gps.speed.knots();
  } else {
        Serial.print(F("INVALID"));
  }

  Serial.print(F("Lat: "));
  Serial.println(latitude, 6);
  Serial.print(F(" Lon: "));
  Serial.println(longitude, 6);
  Serial.print(F(" Alt: "));
  Serial.println(altitude);
   Serial.print(F(" Sats: "));
  Serial.println(satellites);
}

float GPSModule::getLatitude() {
  return latitude;
}

float GPSModule::getLongitude() {
  return longitude;
}

float GPSModule::getAltitude() {
  return altitude;
}
float GPSModule::getCourse() {
  return course;
}

float GPSModule::getSpeedKmph() {
  return speedKmph;
}

float GPSModule::getSpeedKnots() {
  return speedKnots;
}


uint8_t GPSModule::getSatellites() {
  return satellites;
}
