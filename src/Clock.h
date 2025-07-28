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

  // Dostęp do obiektu rtc, aby zarządzać alarmami z pliku .ino
  RTC_DS3231 rtc;

private:
  // RTC_DS3231 rtc; // Przeniesiono do public
};

#endif