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

  String getDateString();
  String getTimeString(bool withSeconds = false);
  
  float getTemperature();

  // Dostęp do obiektu rtc, aby zarządzać alarmami z pliku .ino
  RTC_DS3231 rtc;

private:
  // RTC_DS3231 rtc; // Przeniesiono do public
};

#endif