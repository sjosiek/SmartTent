---
name: SmartTent — przegląd projektu
description: Pełny opis projektu SmartTent: cel, hardware, architektura, kluczowe moduły, przepływ danych
type: project
---

## Co to jest SmartTent

**SmartTent** — inteligentna stacja pogodowa / system sterowania namiotem, zbudowany na **Arduino Mega 2560** (PlatformIO). Główne zadania:
- Monitoring środowiska (temperatura, wilgotność, ciśnienie)
- Autonomiczne śledzenie słońca (solar tracker) za pomocą serw i czujników LDR
- Zapis danych na kartę SD
- Tryb uśpienia (sleep mode) z wybudzeniem przez RTC lub dotyk
- Wyświetlanie danych na LCD 20x4 i wyświetlaczu LED TM1637
- GPS (pozycja, prędkość, kurs, czas UTC)

---

## Hardware

| Komponent | Piny | Interfejs | Rola |
|-----------|------|-----------|------|
| Arduino Mega 2560 | — | — | Główny MCU |
| LCD 20x4 | 20,21 | I2C 0x27 | Główny ekran UI |
| BME280 | 20,21 | I2C 0x76 | Temp/wilgotność/ciśnienie |
| RTC DS3231 | 20,21 | I2C | Zegar, alarmy |
| SD Card | 50–53 | SPI | Logi CSV, config.txt |
| DHT11/22 | 46 | Digital | Temp/wilgotność |
| TM1637 LED | 22,23 | Digital | 4-cyfrowy wyświetlacz czasu |
| GPS | 18,19 | Serial1 | NMEA, czas UTC, pozycja |
| MOSFET power | 4 | Digital | Włączanie peryferiów (sleep) |
| RTC alarm | 2 | INT0 | Wybudzanie z uśpienia |
| Touch sensor | 3 | INT1 | Manualne wybudzanie |
| Servo H | 9 | PWM | Obrót poziomy trackera |
| Servo V | 10 | PWM | Obrót pionowy trackera |
| LDR góra-lewo | A1 | Analog | Czujnik światła TL |
| LDR góra-prawo | A2 | Analog | Czujnik światła TR |
| LDR dół-lewo | A0 | Analog | Czujnik światła BL |
| LDR dół-prawo | A3 | Analog | Czujnik światła BR |
| Joystick 1 | A8,A9 | Analog | Sterowanie |
| Joystick 2 | A10,A11 | Analog | Sterowanie |
| Encoder | 24,25 | Digital | UI navigation |
| Buzzer | 29 | Digital | Dźwięki |
| DIP switch (74HC165) | 30–32 | Digital | 16-bit konfiguracja runtime |

---

## Architektura modułów

Wszystkie moduły są globalnie zinstancjonowane w `main.cpp` (singleton pattern).

### Główna pętla (`main.cpp`)
```
loop():
  CommandHandler.update()   → serial CLI
  ControlPanel.update()     → joystick/encoder
  SunTracker.update()       → maszyna stanów trackera
  GPSModule.update()        → parsowanie NMEA
  PowerManager.update()     → sleep/wake
  LcdDisplay.update()       → renderowanie ekranu
  LedDisplay.update()       → czas na LED
  WDT reset
  IF ACTIVE: read sensors → log SD → update LCD
```

### Konfiguracja (3 poziomy)
1. **Compile-time** — `ProjectConfig.h` (piny, stałe)
2. **DIP switches** — `HardwareConfigReader` (flagi runtime, 8 bitów)
3. **SD card** — `config.txt` (key=value: active_mode_minutes, sensor_update_interval_ms, led_brightness, tracker_update_interval_ms)

---

## Sun Tracker — maszyna stanów (13 stanów)

`STARTUP_WAIT → INIT → [LDR_CALIBRATE] → [SERVO_CALIBRATE] → CENTERING → [SEARCHING] → RUNNING ↔ MANUAL_CONTROL → PARKED`

- **LDR normalizacja:** min/max z EEPROM, różnica góra/dół i lewo/prawo
- **Tolerancja:** jeśli diff > threshold → ruch servo
- **Joystick override:** DIP bit 4 = TRACKER_USE_JOYSTICK

---

## Ekrany LCD (4 ekrany, nawigacja enkoderem)

| Enum | Zawartość |
|------|-----------|
| MAIN | Data, czas, temp BME/DHT, wilgotność, ciśnienie |
| TRACKER | Pozycja servo H/V, 4 wartości LDR |
| GPS | Kurs, prędkość (km/h + knoty), lat/lon, UTC |
| STATUS | Zdrowie modułów (RTC, BME280, DHT, SD, GPS…), auto-scroll 5s |

Nawigacja: obrót enkodera = zmiana ekranu, klik enkodera = powrót do MAIN.

---

## Power Management

- `SystemState`: POWER_UP → ACTIVE → PREPARE_SLEEP → SLEEPING
- `WakeUpSource`: NONE / RTC_ALARM / MANUAL_TOUCH
- Przed snem: INPUT na liniach danych (brak phantom power), MOSFET off
- Po wybudzeniu: przywrócenie peryferiów, reset timera aktywności
- Sleep mode aktywny gdy DIP bit 0 = 1 (domyślnie wyłączony w symulacji)

---

## Biblioteki zewnętrzne (platformio.ini)

- adafruit/RTClib, Adafruit BME280, Adafruit Unified Sensor
- marcoschwartz/LiquidCrystal_I2C
- adafruit/DHT sensor library
- Arduino Servo, SPI, SD, Encoder
- NeoGPS (lib/NeoGPS — lokalna)
- TM1637 (lib/TM1637 — lokalna)
