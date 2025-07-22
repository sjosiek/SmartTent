// Plik: Timer.h
// Prosta klasa pomocnicza do zarządzania zadaniami w czasie
// bez blokowania programu.

#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
public:
  // Konstruktor - ustawia interwał czasowy w milisekundach
  Timer(unsigned long interval);

  // Zwraca 'true', jeśli od ostatniego razu minął określony interwał.
  // Automatycznie resetuje wewnętrzny licznik, gdy zwraca 'true'.
  bool isReady();

  // Pozwala na zmianę interwału w trakcie działania programu
  void setInterval(unsigned long newInterval);
  
  // Resetuje timer, tak jakby zadanie właśnie się wykonało
  void reset();

private:
  unsigned long timerInterval;
  unsigned long previousMillis;
};

#endif