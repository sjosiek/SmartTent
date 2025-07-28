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

float Clock::getTemperature() {
  return rtc.getTemperature();
}