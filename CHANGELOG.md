# Changelog — SmartTent

Wszystkie istotne zmiany w projekcie są dokumentowane w tym pliku.
Format oparty na [Keep a Changelog](https://keepachangelog.com/pl/1.0.0/).
Wersjonowanie według [Semantic Versioning](https://semver.org/lang/pl/): `MAJOR.MINOR.PATCH`

- **MAJOR** — zmiany niekompatybilne (zmiana architektury, pinów, protokołu)
- **MINOR** — nowe funkcje, zachowanie kompatybilności
- **PATCH** — poprawki błędów, kosmetyka

---

## [1.1.0] — 2026-04-10 (devplan002)

### Zmieniono
- GPS: migracja biblioteki `NeoGPS` → `TinyGPS++` (`mikalhart/TinyGPSPlus`)
- GPS: przeniesiony na `Serial2` (pin 16/17 na Mega)
- GPS: parser przepisany — przetwarzanie znak po znaku w każdej iteracji loop()
- GPS: status nie nadpisuje stanu ERROR stanem WARNING
- WDT: tymczasowo wyłączony podczas debugowania GPS

### Dodano
- `platformio.ini`: biblioteka `mikalhart/TinyGPSPlus`

### Znane ograniczenia
- WDT wyłączony — do przywrócenia po stabilizacji GPS

---

## [1.0.1] — 2026-04-10

### Dodano
- `src/Version.h` — centralna definicja wersji firmware (`FIRMWARE_VERSION`, `FIRMWARE_BUILD_INFO`)
- Wersja wyświetlana w Serial monitorze przy boot: `Booting SmartTent v1.0.0...`
- Wersja wyświetlana na LCD podczas boot sequence i ekranie powitalnym

### Dokumentacja
- `WIRING.md` — pełny schemat połączeń wszystkich modułów + moduł MOSFET (2 warianty)
- `README.md` — zaktualizowany do aktualnego stanu kodu, dodana sekcja przywracania pamięci Claude
- `CHANGELOG.md` — wprowadzony (ten plik)
- `docs/devplan001.md` — wprowadzony system devplanów
- `claude/` — pliki pamięci kontekstowej dla Claude AI

---

## [1.0.0] — 2026-04-10

### Bazowa wersja projektu (inicjalna)

#### Funkcje
- Monitoring środowiska: temperatura, wilgotność, ciśnienie (BME280 + DHT11)
- Zegar RTC DS3231 z alarmem i synchronizacją przez Serial (`TIME:` komenda)
- Autonomiczny solar tracker (SunTracker) — 13-stanowa FSM
  - Kalibracja LDR z zapisem do EEPROM
  - Kalibracja serwomechanizmów (pełen zakres 0–180°)
  - Wyszukiwanie słońca (initial search, 20° kroki)
  - Tryb śledzenia i tryb manualny (joystick)
- Moduł GPS (NeoGPS, Serial1) — pozycja, prędkość, kurs, czas UTC
- Wyświetlacz LCD 20x4 — 4 ekrany: MAIN, TRACKER, GPS, STATUS
- Wyświetlacz LED TM1637 — czas z mrugającym dwukropkiem
- Panel sterowania: 2× joystick, enkoder obrotowy, 4× potencjometr, buzzer
- Zapis danych na kartę SD (CSV), konfiguracja przez `config.txt`
- Tryb uśpienia (sleep mode) z wybudzeniem przez RTC lub dotyk (TTP223)
- CLI przez Serial: TIME, SERVO, SERVO_MOVE, SAVE_CONFIG, CALIBRATE_SERVOS
- Melodie na buzzer: startup, X-Files, Wlazł Kotek
- Watchdog Timer (2s)
- Odczyt konfiguracji runtime z DIP switch (74HC165, 16 bitów)

#### Hardware
- MCU: Arduino Mega 2560
- Zasilanie peryferiów: moduł MOSFET sterowany pinem 4

---

*Kolejne wpisy dodawaj na górze (najnowsze pierwsze).*
