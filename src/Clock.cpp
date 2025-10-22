// Plik: Clock.cpp

#include "Clock.h"

Clock::Clock() {}

bool Clock::init() {
  // Uproszczenie: rtc.begin() zwraca bool, więc możemy go zwrócić bezpośrednio.
  return rtc.begin();
}

DateTime Clock::getTime() {
  return rtc.now();
}

void Clock::formatDate(const DateTime& dt, char* buffer, size_t bufferSize) {
  snprintf(buffer, bufferSize, "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());
}

void Clock::formatTime(const DateTime& dt, char* buffer, size_t bufferSize, bool withSeconds) {
  if (withSeconds) {
    snprintf(buffer, bufferSize, "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
  } else {
    snprintf(buffer, bufferSize, "%02d:%02d", dt.hour(), dt.minute());
  }
}

void Clock::configureForAlarm() {
  // To jest kluczowy krok. Aby używać pinu SQW jako wyjścia przerwania,
  // bit INTCN w rejestrze kontrolnym DS3231 musi być ustawiony.
  // Biblioteka RTClib dostarcza do tego celu (nieco myląco nazwaną) opcję DS3231_OFF.
  // Wywołanie writeSqwPinMode z tą wartością konfiguruje pin do pracy w trybie przerwania.
  rtc.writeSqwPinMode(DS3231_OFF);
}

float Clock::getTemperature() {
  float temp = rtc.getTemperature();
  // Dodajemy warunek sprawdzający, czy odczyt jest prawidłowy.
  // Niektóre biblioteki i czujniki w razie błędu zwracają -127.
  if (temp <= INVALID_TEMP) {
    return 0.0f; // Zwracamy bezpieczną wartość w razie błędu
  }
  return temp;
}

void Clock::adjust(const DateTime& dt) {
  rtc.adjust(dt);
}

bool Clock::lostPower() {
  return rtc.lostPower();
}

bool Clock::setAlarm1(const DateTime& dt, Ds3231Alarm1Mode alarm_mode) {
  return rtc.setAlarm1(dt, alarm_mode);
}

bool Clock::alarmFired(uint8_t alarm_num) { return rtc.alarmFired(alarm_num); }

void Clock::clearAlarm(uint8_t alarm_num) { rtc.clearAlarm(alarm_num); }