// Plik: PowerManager.cpp

#include "PowerManager.h"

// ZMIANA: Dwa źródła przerwań i flaga do ich rozróżnienia
volatile WakeUpSource g_wakeUpSource = WakeUpSource::NONE;
void wakeUpISR_RTC() { g_wakeUpSource = WakeUpSource::RTC_ALARM; }
void wakeUpISR_Touch() { g_wakeUpSource = WakeUpSource::MANUAL_TOUCH; }

PowerManager::PowerManager(int powerPin, int rtcAlarmPin, int manualWakeupPin, const uint8_t* dataPins, uint8_t dataPinCount) 
  : _powerPin(powerPin), _rtcAlarmPin(rtcAlarmPin), _manualWakeupPin(manualWakeupPin), _dataPins(dataPins), _dataPinCount(dataPinCount), _activeModeTimer(60000) {
    _currentState = SystemState::POWER_UP;
    // Logika zasilania peryferiów jest stała (sterowanie tranzystorem)
    // Zakładamy, że stan HIGH włącza zasilanie.
    _onState = HIGH;
    _offState = LOW;
}

void PowerManager::begin(Clock& clock, LcdDisplay& lcd, LedDisplay& led, WeatherSensor& sensor, DhtSensor& dht) {
  _clock = &clock;
  _lcd = &lcd;
  _led = &led;
  _sensor = &sensor;
  _dht = &dht;
  
  pinMode(_powerPin, OUTPUT);
  pinMode(_rtcAlarmPin, INPUT_PULLUP); // Alarm RTC zwiera do masy (FALLING)
  // Czujnik dotykowy jest aktywny stanem wysokim, nie wymaga rezystora podciągającego.
  pinMode(_manualWakeupPin, INPUT);
  
  // ZMIANA KRYTYCZNA: Usunięto natychmiastowe wyłączanie zasilania.
  // Funkcja begin() powinna tylko konfigurować, a nie podejmować akcji.
}

bool PowerManager::isAwake() {
  return _currentState == SystemState::ACTIVE;
}

void PowerManager::resetActiveTimer() {
  if (isAwake()) {
    Serial.println(F("Reset timera aktywności..."));
    _activeModeTimer.reset();
  }
}

void PowerManager::update() {
  switch (_currentState) {
    case SystemState::POWER_UP:
      // ZMIANA: Usunięto logikę ponownej inicjalizacji.
      // Teraz tylko zmieniamy stan, aby uniknąć konfliktów.
      Serial.println(F("PowerManager: Stan POWER_UP -> ACTIVE"));
      _activeModeTimer.reset();
      _currentState = SystemState::ACTIVE;
      break;

    case SystemState::ACTIVE:
      if (_activeModeTimer.isReady()) {
        Serial.println(F("Czas aktywności minął. Przygotowuję się do uśpienia."));
        _currentState = SystemState::PREPARE_SLEEP;
      }
      break;

    case SystemState::PREPARE_SLEEP:
      Serial.println(F("Stan: PREPARE_SLEEP"));
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
  Serial.println(F("Włączam zasilanie peryferiów..."));
  // ZMIANA: Używamy uniwersalnej zmiennej _onState
  digitalWrite(_powerPin, _onState);
  delay(200);
}

void PowerManager::deenergizeDataLines() {
  Serial.println(F("De-energetyzacja linii danych w celu uniknięcia 'phantom power'..."));
  
  // Iterujemy po tablicy pinów danych przekazanej w konstruktorze
  // i ustawiamy każdy z nich jako wejście (stan wysokiej impedancji),
  // aby nie mogły one ani dostarczać, ani pobierać prądu.
  for (uint8_t i = 0; i < _dataPinCount; i++) {
    pinMode(_dataPins[i], INPUT);
  }
}

void PowerManager::powerDownPeripherals() {
  Serial.println(F("Odcinam zasilanie peryferiów..."));
  // KROK 1: De-energetyzacja linii danych, aby zapobiec zasilaniu "widmo".
  deenergizeDataLines();
  // KROK 2: Fizyczne odcięcie zasilania VCC za pomocą modułu MOSFET.
  digitalWrite(_powerPin, _offState);
}

void PowerManager::prepareToSleep() {
  _lcd->printSleepMessage();
  delay(3000);
  _lcd->noBacklight();

  // KOLEJNOŚĆ KRYTYCZNA: alarm RTC ustawiamy ZANIM odetniemy zasilanie/I2C.
  // Wcześniej `powerDownPeripherals()` było przed operacjami I2C — co powodowało
  // memory corruption / crash, bo `_clock->setAlarm1()` próbowało gadać z RTC
  // przez deenergetyzowane piny SDA/SCL (patrz log z 1.3.4 → fix w 1.3.6).
  _clock->clearAlarm(1);
  DateTime now = _clock->getTime();
  DateTime future(now + TimeSpan(0, 0, 1, 0)); // Hardcoded: 1 minuta (TODO: config.txt jako sleep_interval_minutes)
  if (!_clock->setAlarm1(future, DS3231_A1_Date)) {
    Serial.println(F("Błąd ustawiania alarmu!"));
  }
  Serial.println(F("Ustawiono alarm na za 1 minute. Dobranoc."));
  Serial.flush(); // wszystko wysłane PRZED odcięciem peryferiów

  // Dopiero teraz bezpiecznie deenergetyzujemy linie danych i tniemy MOSFET.
  powerDownPeripherals();
  delay(100); // krótki delay na wszelki wypadek.
}

void PowerManager::goToSleep() {
  // KRYTYCZNA ZMIANA: Używamy trybu SLEEP_MODE_IDLE zamiast SLEEP_MODE_PWR_DOWN.
  // Tryb PWR_DOWN na ATmega2560 nie może być wybudzony przez przerwanie typu RISING (zbocze narastające),
  // a nasz czujnik dotykowy generuje właśnie taki sygnał.
  // Tryb IDLE zużywa nieco więcej energii, ale pozwala na wybudzenie przez dowolne zbocze przerwania.
  set_sleep_mode(SLEEP_MODE_IDLE);
  g_wakeUpSource = WakeUpSource::NONE;
  sleep_enable();
  
  // Podłączamy OBA przerwania
  attachInterrupt(digitalPinToInterrupt(_rtcAlarmPin), wakeUpISR_RTC, FALLING);
  // Dla czujnika aktywnego stanem wysokim używamy przerwania RISING.
  attachInterrupt(digitalPinToInterrupt(_manualWakeupPin), wakeUpISR_Touch, RISING);
  
  sleep_cpu();
  sleep_disable();
  detachInterrupt(digitalPinToInterrupt(_rtcAlarmPin));
  detachInterrupt(digitalPinToInterrupt(_manualWakeupPin));
}

void PowerManager::handleWakeUp() {

  if (g_wakeUpSource != WakeUpSource::NONE) {
    if (g_wakeUpSource == WakeUpSource::RTC_ALARM) {
      _clock->clearAlarm(1);
      Serial.println(F("Obudził mnie ALARM. Uruchamiam system na cykl pracy."));
      _currentState = SystemState::POWER_UP;
      _wakeEventPending = true;
    } else if (g_wakeUpSource == WakeUpSource::MANUAL_TOUCH) {
      Serial.println(F("Obudził mnie DOTYK. Uruchamiam system."));
      _currentState = SystemState::POWER_UP;
      _wakeEventPending = true;
    }
  }
}

void PowerManager::forceSleep() {
  Serial.println(F("Force sleep — wymuszone przez long press touch."));
  _currentState = SystemState::PREPARE_SLEEP;
}

bool PowerManager::consumeWakeEvent() {
  if (_wakeEventPending) {
    _wakeEventPending = false;
    return true;
  }
  return false;
}

void PowerManager::setActiveModeDuration(uint32_t minutes) {
  // Konwertujemy minuty na milisekundy i ustawiamy interwał timera
  uint32_t durationMs = minutes * 60 * 1000;
  _activeModeTimer.setInterval(durationMs);
}