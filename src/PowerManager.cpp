// Plik: PowerManager.cpp

#include "PowerManager.h"

volatile bool g_interruptFired = false;
void wakeUpISR() { g_interruptFired = true; }

// ZMIANA: Konstruktor ustawia stany _onState i _offState na podstawie parametru
PowerManager::PowerManager(int powerPin, int interruptPin, LogicLevel logic) 
  : _powerPin(powerPin), _interruptPin(interruptPin), _activeModeTimer(60000) {
    _currentState = SystemState::POWER_UP;
    if (logic == LogicLevel::ACTIVE_LOW) {
      _onState = LOW;
      _offState = HIGH;
    } else { // ACTIVE_HIGH
      _onState = HIGH;
      _offState = LOW;
    }
}

void PowerManager::begin(Clock& clock, LcdDisplay& lcd, LedDisplay& led, WeatherSensor& sensor) {
  _clock = &clock;
  _lcd = &lcd;
  _led = &led;
  _sensor = &sensor;

  pinMode(_powerPin, OUTPUT);
  pinMode(_interruptPin, INPUT_PULLUP);
  
  powerDownPeripherals();
}

bool PowerManager::isAwake() {
  return _currentState == SystemState::ACTIVE;
}

void PowerManager::resetActiveTimer() {
  if (isAwake()) {
    Serial.println("Reset timera aktywności...");
    _activeModeTimer.reset();
  }
}

void PowerManager::update() {
  switch (_currentState) {
    case SystemState::POWER_UP:
      Serial.println("Stan: POWER_UP");
      powerUpPeripherals();
      _lcd->init();
      _led->init(10);
      if (!_sensor->init()) {
        Serial.println("Błąd inicjalizacji czujnika BME280 w PowerManager!");
      } else {
        Serial.println("Czujnik BME280 OK (PowerManager).");
      }
      _lcd->printWelcomeMessage();
      delay(2000);
      _activeModeTimer.reset();
      _currentState = SystemState::ACTIVE;
      break;

    case SystemState::ACTIVE:
      if (_activeModeTimer.isReady()) {
        Serial.println("Czas aktywności minął. Przygotowuję się do uśpienia.");
        _currentState = SystemState::PREPARE_SLEEP;
      }
      break;

    case SystemState::PREPARE_SLEEP:
      Serial.println("Stan: PREPARE_SLEEP");
      prepareToSleep();
      _currentState = SystemState::SLEEPING;
      break;

    case SystemState::SLEEPING:
      goToSleep();
      handleWakeUp();
      break;
  }
}

void PowerManager::powerUpPeripherals() {
  Serial.println("Włączam zasilanie peryferiów...");
  // ZMIANA: Używamy uniwersalnej zmiennej _onState
  digitalWrite(_powerPin, _onState);
  delay(200);
}

void PowerManager::powerDownPeripherals() {
  Serial.println("Odcinam zasilanie peryferiów...");
  // ZMIANA: Używamy uniwersalnej zmiennej _offState
  digitalWrite(_powerPin, _offState);
}

void PowerManager::prepareToSleep() {
  _lcd->printSleepMessage();
  delay(1000);
  _lcd->noBacklight();
  powerDownPeripherals();

  DateTime now = _clock->getTime();
  DateTime future(now + TimeSpan(0, 0, 5, 0)); // 5 minut
  if (!_clock->rtc.setAlarm1(future, DS3231_A1_Second)) {
    Serial.println("Błąd ustawiania alarmu!");
  }
  Serial.println("Ustawiono alarm na za 5 minut. Dobranoc.");
  delay(100);
}

void PowerManager::goToSleep() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  g_interruptFired = false;
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(_interruptPin), wakeUpISR, FALLING);
  sleep_cpu();
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(_interruptPin));
}

void PowerManager::handleWakeUp() {
  Serial.println("Pobudka!");
  if (g_interruptFired) {
    if (_clock->rtc.alarmFired(1)) {
      _clock->rtc.clearAlarm(1);
      Serial.println("Obudził mnie ALARM. Uruchamiam system na cykl pracy.");
      // ZMIANA: Przechodzimy do POWER_UP, aby wykonać pełny cykl pracy, a nie od razu spać.
      _currentState = SystemState::POWER_UP;
    } else {
      Serial.println("Obudził mnie PRZYCISK. Uruchamiam system.");
      _currentState = SystemState::POWER_UP;
    }
  }
}