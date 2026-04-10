---
name: SmartTent — mapa plików źródłowych
description: Lista wszystkich plików src/ z opisami funkcji, kluczowych klas i gdzie szukać co
type: project
---

## Pliki źródłowe (src/)

### Główne
| Plik | Rola |
|------|------|
| `main.cpp` | Entry point, globalne instancje, setup(), loop(), WDT, SensorData |
| `ProjectConfig.h` | Wszystkie stałe pinów, structs: Configuration, RuntimeFlags, enum DipSwitchBits |
| `DeviceStatus.h` | enum ModuleHealth, struct ModuleStatus |
| `SensorData.h` | Struct zbiorczy dla wszystkich odczytów sensorów + dane GPS + sun tracker |

### Zegar / RTC
| Plik | Rola |
|------|------|
| `Clock.h/cpp` | Wrapper RTC DS3231: init, getTime, adjust, setAlarm1, formatDate, formatTime |

### Sensory środowiskowe
| Plik | Rola |
|------|------|
| `WeatherSensor.h/cpp` | BME280: temp/wilgotność/ciśnienie, auto-reinit przy błędzie I2C |
| `DhtSensor.h/cpp` | DHT11/22: temp/wilgotność, min 2s między odczytami |

### Wyświetlacze
| Plik | Rola |
|------|------|
| `LcdDisplay.h/cpp` | LCD 20x4, enum LcdScreen, 4 ekrany, nextScreen/previousScreen, auto-paginacja STATUS |
| `LedDisplay.h/cpp` | TM1637 4-cyfry, czas z mrugającym dwukropkiem, brightness |

### Sun Tracker
| Plik | Rola |
|------|------|
| `SunTracker.h/cpp` | 13-stanowa FSM, śledzenie słońca, kalibracja LDR/servo, EEPROM, manual control |
| `SmoothServo.h/cpp` | Non-blocking servo z płynnym ruchem, setTargetPosition, kalibracja 0→180° |

### GPS
| Plik | Rola |
|------|------|
| `GPSModule.h/cpp` | Parser NeoGPS (Serial1), pozycja/alt/speed/heading/satellites/UTC datetime |

### Power Management
| Plik | Rola |
|------|------|
| `PowerManager.h/cpp` | FSM: POWER_UP/ACTIVE/PREPARE_SLEEP/SLEEPING, ISR dla RTC i touch, MOSFET |

### Storage
| Plik | Rola |
|------|------|
| `SDCard.h/cpp` | CSV logi sensorów, parsing config.txt (key=value), detekcja pełnej karty |

### Serial CLI
| Plik | Rola |
|------|------|
| `CommandHandler.h/cpp` | Komendy: TIME:, SERVO:, SERVO_MOVE:, SAVE_CONFIG, CALIBRATE_SERVOS |

### Panel sterowania
| Plik | Rola |
|------|------|
| `ControlPanel.h/cpp` | 2x joystick (raw+mapped, dead zone, 8 kierunków), encoder, 4x pot, buzzer |
| `DebouncedButton.h/cpp` | Debouncowanie przycisków, enum ActiveState, wasPressed/isPressed |
| `Timer.h/cpp` | Non-blocking interval timer: isReady(), setInterval(), reset() |

### Audio
| Plik | Rola |
|------|------|
| `SoundPlayer.h/cpp` | Melodie na buzzer: startup beeps, X-Files theme, Wlazł Kotek |

### Konfiguracja hardware
| Plik | Rola |
|------|------|
| `HardwareConfigReader.h/cpp` | 74HC165 shift register, 16-bit DIP switches → RuntimeFlags |

---

## Gdzie szukać co

| Chcę zmienić... | Plik |
|-----------------|------|
| Ekrany LCD | `LcdDisplay.cpp` (_drawMainScreen, _drawTrackerScreen, _drawGpsScreen, _drawStatusScreen) |
| Częstotliwość sensorów | `ProjectConfig.h` + `config.txt` na SD |
| Algorytm trackera słońca | `SunTracker.h/cpp` (SunTrackerConfig: tolerance, interval) |
| Komendy seryjne | `CommandHandler.cpp` (update()) |
| Melodie / dźwięki | `SoundPlayer.cpp` (TuneNote arrays) |
| Przypisanie pinów | `ProjectConfig.h` (wszystkie PIN_* stałe) |
| Sleep/wake | `PowerManager.cpp` (przejścia stanów) |
| DIP switch bity | `ProjectConfig.h` (enum DipSwitchBits) |
| Pozycja serw | `SmoothServo.h/cpp` |

---

## Pliki zewnętrzne (ext/)

| Plik | Rola |
|------|------|
| `serial-port.py` | Narzędzie Python do komunikacji szeregowej |
| `sync-time.py` | Synchronizacja czasu RTC przez serial |
| `wschod_zachod.csv` | Dane wschodu/zachodu słońca |
| `requirements.txt` | Zależności Python |

## README.md — uwaga

README.md był wcześniej nieaktualny (opisywał wczesną wersję projektu). Został zaktualizowany do stanu kodu w tej sesji. Source of truth zawsze jest kod w `src/`.
