#ifndef SENSORDATA_H
#define SENSORDATA_H

#include <Arduino.h>
struct SensorData {
  char dateStr[11];    // "YYYY-MM-DD\0"
  char timeForLcd[9];  // "HH:MM:SS\0"
  uint8_t hour;
  uint8_t minute;
  float temp_bme;
  float hum_bme;
  float pressure_bme;
  float temp_rtc;
  float temp_dht;
  float hum_dht;
  // ZMIANA: Pola dla ekranu trackera
  int servo_h_pos;
  int servo_v_pos;
  int ldr_tl;
  int ldr_tr;
  int ldr_dl;
  int ldr_dr;
  // ZMIANA: Pola dla ekranu GPS
  bool gps_is_valid;
  float gps_lat;
  float gps_lon;
  float gps_alt;
  uint8_t gps_sats;
  bool gps_time_valid;
  uint16_t gps_year;
  uint8_t gps_month;
  uint8_t gps_day;
  uint8_t gps_hour;
  uint8_t gps_minute;
  uint8_t gps_second;
};

#endif