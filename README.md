# SmartTent

Projekt inteligentnego namiotu / stacji pogodowej opartej na **Arduino Mega 2560** (PlatformIO). System monitoruje warunki otoczenia (temperatura, wilgotność, ciśnienie), zarządza energią w trybie oszczędzania, steruje modułem śledzenia słońca (`SunTracker`) oraz rejestruje dane na karcie SD.

---

## Claude AI — przywracanie pamięci na nowym komputerze

Katalog [`claude/`](claude/) zawiera pliki pamięci kontekstowej dla Claude Code. Dzięki nim asystent ma pełną wiedzę o projekcie od razu po otwarciu — bez ponownej analizy kodu.

**Aby przywrócić pamięć na nowym komputerze:**

1. Znajdź lub utwórz katalog pamięci projektu dla Claude. Ścieżka zależy od nazwy katalogu roboczego:
   ```
   # Windows
   %USERPROFILE%\.claude\projects\<nazwa-katalogu-projektu>\memory\

   # macOS / Linux
   ~/.claude/projects/<nazwa-katalogu-projektu>/memory/
   ```
   Nazwa katalogu projektu to ścieżka do repo z zamienionymi separatorami na `-`, np.:
   `c:\PRIV\DEV\SmartTent` → `c--PRIV-DEV-SmartTent`

2. Skopiuj zawartość katalogu `claude/` do powyższej ścieżki:
   ```bash
   cp claude/* ~/.claude/projects/c--PRIV-DEV-SmartTent/memory/
   ```

3. Gotowe — Claude będzie miał kontekst projektu przy następnej sesji.

---

## 1. Mapa połączeń (Arduino Mega 2560)

### Magistrale

| Magistrala | Piny | Urządzenia |
|------------|------|-----------|
| **I2C** (SDA/SCL) | 20, 21 | LCD 20x4 (0x27), BME280 (0x76), RTC DS3231 |
| **SPI** (MOSI/MISO/SCK/CS) | 51, 50, 52, **53** | Karta SD |
| **Serial1** (RX1/TX1) | 19, 18 | Moduł GPS |

### Piny cyfrowe i analogowe

| Pin | Opis | Moduł |
|-----|------|-------|
| **2** | Alarm RTC (SQW) — Przerwanie 0 | RTC DS3231 |
| **3** | Czujnik dotykowy TTP223 — Przerwanie 1 | Power Manager |
| **4** | Sterowanie zasilaniem peryferiów (MOSFET gate) | Power Manager |
| **9** | Serwomechanizm poziomy (H) | Sun Tracker |
| **10** | Serwomechanizm pionowy (V) | Sun Tracker |
| **13** | Wbudowana dioda LED (heartbeat / sygnalizacja błędu) | System |
| **22** | CLK wyświetlacza LED TM1637 | LED Display |
| **23** | DIO wyświetlacza LED TM1637 | LED Display |
| **24** | Przycisk joysticka 1 (SW) | Control Panel |
| **25** | Przycisk joysticka 2 (SW) | Control Panel |
| **26** | Enkoder obrotowy (DT) | Control Panel |
| **27** | Enkoder obrotowy (CLK) | Control Panel |
| **28** | Enkoder obrotowy (SW) | Control Panel |
| **29** | Buzzer | Control Panel |
| **30** | DIP switch 74HC165 (LATCH) | HardwareConfigReader |
| **31** | DIP switch 74HC165 (CLK) | HardwareConfigReader |
| **32** | DIP switch 74HC165 (DATA) | HardwareConfigReader |
| **46** | Dane czujnika DHT11/22 | DhtSensor |
| **A0** | Fotorezystor Dół-Lewo (DL) | Sun Tracker |
| **A1** | Fotorezystor Góra-Lewo (TL) | Sun Tracker |
| **A2** | Fotorezystor Góra-Prawo (TR) | Sun Tracker |
| **A3** | Fotorezystor Dół-Prawo (DR) | Sun Tracker |
| **A8** | Joystick 1 — oś X | Control Panel |
| **A9** | Joystick 1 — oś Y | Control Panel |
| **A10** | Joystick 2 — oś X | Control Panel |
| **A11** | Joystick 2 — oś Y | Control Panel |
| **A12–A15** | Potencjometry 1–4 | Control Panel |

---

## 2. Konfiguracja i sterowanie

System obsługuje trzy poziomy konfiguracji.

### 2.1. Przełączniki DIP (74HC165) — flagi runtime

Odczytywane przy starcie przez `HardwareConfigReader`. 16 bitów (8 wolnych na przyszłość).

| Bit | Stała `DipSwitchBits` | Opis |
|-----|-----------------------|------|
| 0 | `DIP_SLEEP_MODE_ENABLED` | Włącza tryb oszczędzania energii |
| 1 | `DIP_TRACKER_LDR_CALIBRATION` | Kalibracja fotorezystorów przy starcie |
| 2 | `DIP_TRACKER_SERVO_CALIBRATION` | Kalibracja serw przy starcie (przejazd 0–180°) |
| 3 | `DIP_TRACKER_INITIAL_SEARCH` | Aktywne wyszukiwanie słońca po starcie |
| 4 | `DIP_TRACKER_USE_JOYSTICK` | Manualne sterowanie joystickiem |
| 5 | `DIP_TRACKER_LDR_SENSORS_CONNECTED` | Fotorezystory są fizycznie podłączone |
| 6 | `DIP_TRACKER_ENABLE_SERVO_MOVEMENT` | Globalna blokada ruchu serw trackera |
| 7 | `DIP_TRACKER_ENABLE_DEBUG_PRINT` | Szczegółowe logi SunTracker na Serial |
| 8–15 | — | Wolne |

> W kodzie (`main.cpp`) można symulować stan DIP przez przypisanie wartości do `dipState` — przydatne w testach bez fizycznego modułu.

### 2.2. Konfiguracja w kodzie (`main.cpp` + `SunTracker`)

Wymagają rekompilacji. Dotyczą trackera (`SunTrackerConfig`):

| Parametr | Wartość domyślna | Opis |
|----------|-----------------|------|
| `servoVMinAngle` / `servoVMaxAngle` | 0 / 90 | Zakres kąta serwa pionowego |
| `servoHMinAngle` / `servoHMaxAngle` | 0 / 180 | Zakres kąta serwa poziomego |
| `defaultServoSpeed` | 100 | Prędkość ruchu serw (większa = wolniej) |
| `defaultTolerance` | 50 | Czułość trackera (min. różnica LDR wywołująca ruch) |

### 2.3. Plik `config.txt` na karcie SD

Wczytywany przy starcie, format `klucz=wartość`. Nie wymaga rekompilacji.

| Klucz | Domyślna | Opis |
|-------|----------|------|
| `active_mode_minutes` | `5` | Czas aktywności po wybudzeniu (minuty) |
| `sensor_update_interval_ms` | `1000` | Interwał odczytu sensorów i zapisu na SD (ms) |
| `led_brightness` | `7` | Jasność wyświetlacza TM1637 (0–15) |
| `tracker_update_interval_ms` | `300000` | Interwał aktualizacji trackera (ms) |

---

## 3. Sterowanie przez port szeregowy (9600 baud)

| Komenda | Przykład | Opis |
|---------|---------|------|
| `TIME:YYYY-MM-DD,HH:MM:SS` | `TIME:2025-06-01,12:00:00` | Ustawia czas RTC |
| `SERVO:[index]:[kąt]` | `SERVO:0:120` | Ustawia serwo (0=H, 1=V) na zadany kąt |
| `SERVO_MOVE:[index]:[dir]` | `SERVO_MOVE:1:LEFT` | Przesuwa serwo o jeden krok |
| `SAVE_CONFIG` | — | Zapisuje konfigurację na kartę SD |
| `CALIBRATE_SERVOS` | — | Uruchamia sekwencję kalibracji serw |

---

## 4. Ekrany LCD (20x4)

Nawigacja: enkoder obrotowy (obrót = zmiana ekranu, klik = powrót do MAIN).

| Ekran | Zawartość |
|-------|-----------|
| **MAIN** | Data, czas, temperatura BME280/DHT, wilgotność, ciśnienie |
| **TRACKER** | Pozycja servo H/V, wartości 4 fotorezystorów (TL, TR, DL, DR) |
| **GPS** | Kurs, prędkość (km/h i węzły), współrzędne, czas UTC |
| **STATUS** | Stan modułów (RTC, BME280, DHT11, SD, GPS, Control Panel, Servos) |

---

## 5. Sun Tracker — maszyna stanów

```
STARTUP_WAIT → INIT → [LDR_CALIBRATE] → [SERVO_CALIBRATE] → CENTERING → [SEARCHING] → RUNNING ↔ MANUAL_CONTROL
                                                                                           ↓
                                                                                         PARKED
```

- Kalibracja LDR zapisywana/wczytywana z EEPROM (adres 0, magic key `0x5453`)
- Kroki wyszukiwania: 20° w osi H i V
- Normalizacja LDR: wartości raw → 0–1023 według min/max z kalibracji

---

## 6. Power Manager

- `ACTIVE` → `PREPARE_SLEEP` → `SLEEPING` po upływie `active_mode_minutes`
- Wybudzenie: alarm RTC (INT0) lub czujnik dotykowy TTP223 (INT1)
- Przed snem: de-energetyzacja linii danych (INPUT bez pull-up), wyłączenie MOSFET
- Watchdog Timer: 2 s (reset systemu przy zawieszeniu)
