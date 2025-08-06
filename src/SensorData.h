#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <Arduino.h>

struct SensorData {
  String dateStr;
  String timeForLcd;
  String timeForLed;
  float temp_bme;
  float hum_bme;
  float pressure_bme;
  float temp_rtc;
  float temp_dht;
  float hum_dht;
  int servo1_pos;
  int servo2_pos;
};

#endif