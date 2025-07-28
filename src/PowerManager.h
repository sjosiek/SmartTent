// Plik: PowerManager.h

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <avr/sleep.h>
#include "Clock.h"
#include "LcdDisplay.h"
#include "LedDisplay.h"
#include "WeatherSensor.h"
#include "Timer.h"

enum class LogicLevel {
  ACTIVE_LOW,
  ACTIVE_HIGH
};

enum class SystemState {
  POWER_UP,
  ACTIVE,
  PREPARE_SLEEP,
  SLEEPING
};

class PowerManager {
public:
  PowerManager(int powerPin, int interruptPin, LogicLevel logic); // Zostaje bez zmian
  void begin(Clock& clock, LcdDisplay& lcd, LedDisplay& led, WeatherSensor& sensor);
  void update();
  bool isAwake();
  void resetActiveTimer();
  
  // ZMIANA: Te metody stają się publiczne, aby można było ich użyć w setup()
  void powerUpPeripherals();
  void powerDownPeripherals();

private:
  // ZMIANA: Usunięto stąd powerUpPeripherals() i powerDownPeripherals()
  void goToSleep();
  void handleWakeUp();
  void prepareToSleep();

  int _powerPin;
  int _interruptPin;
  SystemState _currentState;
  Timer _activeModeTimer;
  
  uint8_t _onState;
  uint8_t _offState;
  
  Clock* _clock;
  LcdDisplay* _lcd;
  LedDisplay* _led;
  WeatherSensor* _sensor;
};

#endif