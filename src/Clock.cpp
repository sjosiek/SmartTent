// Plik: Clock.cpp

#include "Clock.h"

Clock::Clock() {}

bool Clock::init() {
  if (!rtc.begin()) {
    return false;
  }
  return true;
}

DateTime Clock::getTime() {
  return rtc.now();
}

String Clock::formatDate(const DateTime& dt) {
  char buffer[11]; // Bufor na "YYYY-MM-DD\0"
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", dt.year(), dt.month(), dt.day());
  return String(buffer);
}

String Clock::formatTime(const DateTime& dt, bool withSeconds) {
  if (withSeconds) {
    char buffer[9]; // Bufor na "HH:MM:SS\0"
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", dt.hour(), dt.minute(), dt.second());
    return String(buffer);
  } else {
    char buffer[6]; // Bufor na "HH:MM\0"
    snprintf(buffer, sizeof(buffer), "%02d:%02d", dt.hour(), dt.minute());
    return String(buffer);
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