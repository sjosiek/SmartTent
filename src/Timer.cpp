// Plik: Timer.cpp

#include "Timer.h"

Timer::Timer(unsigned long interval) {
  timerInterval = interval;
  previousMillis = 0; // Inicjalizujemy na 0, by pierwsze uruchomienie było poprawne
}

bool Timer::isReady() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= timerInterval) {
    // Czas minął, resetujemy timer i zwracamy prawdę
    previousMillis = currentMillis;
    return true;
  }
  // Czas jeszcze nie minął
  return false;
}

void Timer::setInterval(unsigned long newInterval) {
  timerInterval = newInterval;
}

void Timer::reset() {
  previousMillis = millis();
}