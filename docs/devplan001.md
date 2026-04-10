# devplan001 — Inicjalna wersja 1.0.0

| Pole | Wartość |
|------|---------|
| **Nr** | 001 |
| **Data** | 2026-04-10 |
| **Wersja docelowa** | 1.0.0 |
| **Status** | ✅ ZREALIZOWANY |
| **Autor** | Sebastian Josiek |

## Opis

Ustanowienie bazowej wersji projektu SmartTent. Projekt był rozwijany bez formalnego wersjonowania — ten devplan dokumentuje stan jako punkt startowy dla dalszego development.

## Co wchodzi w skład v1.0.0

- Pełna funkcjonalność solar trackera (FSM 13 stanów)
- Monitoring środowiska (BME280, DHT11, RTC temp)
- GPS (NeoGPS, Serial1)
- LCD 4 ekrany + LED TM1637
- Panel sterowania (2× joystick, enkoder, 4× pot, buzzer)
- SD card (logi CSV, config.txt)
- Sleep mode (RTC alarm + TTP223 touch wakeup)
- Serial CLI
- DIP switch runtime config (74HC165)
- Watchdog Timer

## Pliki dodane w ramach tego devplanu

| Plik | Opis |
|------|------|
| `src/Version.h` | Definicja wersji firmware (`FIRMWARE_VERSION "1.0.0"`) |
| `CHANGELOG.md` | Historia zmian projektu |
| `WIRING.md` | Schemat połączeń wszystkich modułów |
| `README.md` | Zaktualizowana dokumentacja (zgodna z kodem) |
| `docs/devplan001.md` | Ten plik |
| `claude/` | Pliki pamięci kontekstowej dla Claude AI |

## Znane bugi w tej wersji

| ID | Plik | Opis | Priorytet |
|----|------|------|-----------|
| BUG1 | `main.cpp:121` | Błędne piny I2C (A4,A5 zamiast 20,21) w liście de-energetyzacji | Wysoki (ujawni się przy sleep mode) |
| BUG2 | `SunTracker.cpp:50` | Podwójne `controlPanel.update()` | Niski (działa w praktyce) |
| BUG3 | `SunTracker.cpp` | Niezainicjalizowane `ldrXxxVal` w konstruktorze | Średni (śmieci na ekranie TRACKER na starcie) |
| BUG4 | `LcdDisplay.cpp:194` | Debug prints w `_drawStatusScreen()` bez guarda | Niski |
| BUG5 | `PowerManager.cpp:114` | Sleep alarm hardcoded 1 minuta (nie z config) | Średni |
| BUG6 | `main.cpp:27` | Zły pin DHT w komentarzu (6 zamiast 46) | Kosmetyczny |

## Następny devplan

devplan002 — do ustalenia (sugestia: naprawa BUG1 + BUG3 przed włączeniem sleep mode)
