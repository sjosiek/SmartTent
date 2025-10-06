// Plik: Clock.h

#ifndef CLOCK_H
#define CLOCK_H

#include <RTClib.h>
#include <Arduino.h>

class Clock {
public:
  Clock();
  bool init();
  DateTime getTime() const;

  static String formatDate(const DateTime& dt);
  static String formatTime(const DateTime& dt, bool withSeconds = false);

  void configureForAlarm();
  float getTemperature() const;

  // Metody opakowujące funkcje RTC dla lepszej hermetyzacji
  void adjust(const DateTime& dt);
  bool lostPower() const;
  bool setAlarm1(const DateTime& dt, Ds3231Alarm1Mode alarm_mode);
  bool alarmFired(uint8_t alarm_num) const;
  void clearAlarm(uint8_t alarm_num);

private:
  static constexpr float INVALID_TEMP = -127.0f; // Wartość błędu zwracana przez niektóre czujniki
  RTC_DS3231 rtc;
};

#endif