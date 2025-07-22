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

String Clock::getDateString() {
  DateTime now = rtc.now();
  char buffer[11]; // Bufor na "YYYY-MM-DD\0"
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", now.year(), now.month(), now.day());
  return String(buffer);
}

String Clock::getTimeString(bool withSeconds) {
  DateTime now = rtc.now();
  if (withSeconds) {
    char buffer[9]; // Bufor na "HH:MM:SS\0"
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    return String(buffer);
  } else {
    char buffer[6]; // Bufor na "HH:MM\0"
    snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
    return String(buffer);
  }
}

float Clock::getTemperature() {
  return rtc.getTemperature();
}