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
  powerDownPeripherals();

  // KROK 1: Upewnij się, że flaga poprzedniego alarmu jest wyczyszczona.
  // To kluczowe, aby pin SQW mógł ponownie przejść w stan wysoki i wygenerować
  // nowe zbocze opadające przy następnym alarmie.
  _clock->clearAlarm(1);
  DateTime now = _clock->getTime();
  // Ustawiamy alarm na 5 minut w przyszłość, zgodnie z komunikatem.
  DateTime future(now + TimeSpan(0, 0, 1, 0));
  // Używamy DS3231_A1_Date, aby alarm zadziałał o konkretnej dacie i godzinie.
  // Poprzedni tryb (DS3231_A1_Second) powodował, że alarm dzwonił co minutę,
  // gdy tylko sekundy się zgadzały, co nie było zamierzonym zachowaniem.
  if (!_clock->setAlarm1(future, DS3231_A1_Date)) {
    Serial.println(F("Błąd ustawiania alarmu!"));
  }
  Serial.println(F("Ustawiono alarm na za 1 minutę w przyszłość. Dobranoc."));
  delay(100); // Krótki delay na wszelki wypadek.
  Serial.flush(); // KLUCZOWA ZMIANA: Czekamy, aż wszystkie dane zostaną wysłane przez port szeregowy.
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
    } else if (g_wakeUpSource == WakeUpSource::MANUAL_TOUCH) {
      Serial.println(F("Obudził mnie DOTYK. Uruchamiam system."));
      _currentState = SystemState::POWER_UP;
    }
  }
}

void PowerManager::setActiveModeDuration(uint32_t minutes) {
  // Konwertujemy minuty na milisekundy i ustawiamy interwał timera
  uint32_t durationMs = minutes * 60 * 1000;
  _activeModeTimer.setInterval(durationMs);
}