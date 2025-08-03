// Plik: Clock.h

#ifndef CLOCK_H
#define CLOCK_H

#include <RTClib.h>
#include <Arduino.h>

class Clock {
public:
  Clock();
  bool init();
  DateTime getTime();

  static String formatDate(const DateTime& dt);
  static String formatTime(const DateTime& dt, bool withSeconds = false);

  float getTemperature();

  // Metody opakowujące funkcje RTC dla lepszej hermetyzacji
  void adjust(const DateTime& dt);
  bool lostPower();
  bool setAlarm1(const DateTime& dt, Ds3231Alarm1Mode alarm_mode);
  bool alarmFired(uint8_t alarm_num);
  void clearAlarm(uint8_t alarm_num);

private:
  RTC_DS3231 rtc;
};

#endif