// Plik: PowerManager.h

#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <avr/sleep.h>
#include "Clock.h"
#include "LcdDisplay.h"
#include "LedDisplay.h"
#include "WeatherSensor.h"
#include "DhtSensor.h"
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

// Dodajemy enum, aby wiedzieć, co wybudziło system
enum class WakeUpSource {
  NONE,
  RTC_ALARM,
  MANUAL_TOUCH
};

class PowerManager {
public:
  PowerManager(int powerPin, int rtcAlarmPin, int manualWakeupPin, const uint8_t* dataPins, uint8_t dataPinCount);
  void begin(Clock& clock, LcdDisplay& lcd, LedDisplay& led, WeatherSensor& sensor, DhtSensor& dht);
  void update();
  bool isAwake();
  void resetActiveTimer();
  void setActiveModeDuration(uint32_t minutes);
  
  // Metoda publiczna, aby można było jej użyć w setup() w trybie bez uśpienia
  void powerUpPeripherals();

private:
  void goToSleep();
  void powerDownPeripherals(); // ZMIANA: Ta metoda jest używana tylko wewnętrznie
  void handleWakeUp();
  void prepareToSleep();
  void deenergizeDataLines();

  int _powerPin;
  int _rtcAlarmPin;
  int _manualWakeupPin;
  const uint8_t* _dataPins;
  uint8_t _dataPinCount;
  SystemState _currentState;
  Timer _activeModeTimer;
  
  uint8_t _onState;
  uint8_t _offState;
  
  Clock* _clock;
  LcdDisplay* _lcd;
  LedDisplay* _led;
  WeatherSensor* _sensor;
  DhtSensor* _dht;
};

#endif