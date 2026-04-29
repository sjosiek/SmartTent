# Changelog — SmartTent

Wszystkie istotne zmiany w projekcie są dokumentowane w tym pliku.
Format oparty na [Keep a Changelog](https://keepachangelog.com/pl/1.0.0/).
Wersjonowanie według [Semantic Versioning](https://semver.org/lang/pl/): `MAJOR.MINOR.PATCH`

- **MAJOR** — zmiany niekompatybilne (zmiana architektury, pinów, protokołu)
- **MINOR** — nowe funkcje, zachowanie kompatybilności
- **PATCH** — poprawki błędów, kosmetyka

---

## [1.3.6] — 2026-04-29

### Naprawiono
- **Latent bug w `PowerManager::prepareToSleep()`** — `powerDownPeripherals()` (deenergetyzacja I2C SDA/SCL na `INPUT`) było wywoływane PRZED operacjami RTC (`clearAlarm()`, `getTime()`, `setAlarm1()`). To powodowało, że `_clock->setAlarm1()` próbowało rozmawiać z RTC przez wyłączoną magistralę I2C → memory corruption / crash. Naprawione w 1.3.5 przez guard `update()`, ale faktyczna kolejność operacji była nadal niepoprawna — przy włączonym sleep mode (DIP bit 0 = 1) bug pojawiłby się ponownie. Teraz: alarm RTC jest ustawiany ZANIM peryferia są deenergetyzowane.

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.3.5 → 1.3.6
- `src/PowerManager.cpp` — zamieniona kolejność w `prepareToSleep()`: najpierw `_clock->setAlarm1()` + `Serial.flush()`, potem `powerDownPeripherals()`

---

## [1.3.5] — 2026-04-29

### Naprawiono
- **System restartował się co ~5 minut mimo `sleepModeEnabled=false`** — `PowerManager::update()` było wywoływane bezwarunkowo w `loop()`, więc maszyna stanów po `activeModeMinutes` przechodziła w `PREPARE_SLEEP`. `prepareToSleep()` po `powerDownPeripherals()` (który ustawia I2C piny na `INPUT`) próbował dalej rozmawiać z RTC przez I2C → memory corruption / reset. Teraz `powerManager.update()` jest wywoływane tylko gdy `g_runtimeFlags.sleepModeEnabled` jest `true`. Diagnostyka z logu Serial 1.3.4: `"Czas aktywności minął. Przygotowuję się do uśpienia." → "Stan: �␀␀␀..."` → reset.

### Dodano
- **Sanity check Reset cause** — zarezerwowane bity MCUSR (5, 6, 7) muszą być `0`. Jeśli któryś jest ustawiony, `g_mcusrMirror` zawiera śmieci (bootloader stk500v2 nie kopiuje MCUSR do `r2` jak Optiboot). Wtedy zamiast wprowadzać w błąd, pokazujemy `Reset: (invalid)` na LCD i `Reset cause: (invalid - bootloader)` na Serial. Surowy MCUSR=0xXX nadal pokazany w 4. linii LCD.

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.3.4 → 1.3.5
- `src/main.cpp` — guard `if (g_runtimeFlags.sleepModeEnabled)` wokół `powerManager.update()`; sanity check `(resetFlags & 0xE0) == 0` w diagnostyce Reset cause (Serial + LCD)

### Rekomendacja
Aby Reset cause działał wiarygodnie po jakimś czasie — wgrać Optiboot na Mega 2560 przez ISP (np. drugą Arduino jako programmer + IDE → "Burn Bootloader"). Po Optiboot wartość MCUSR jest zachowana w `r2` zgodnie z naszą implementacją `.init0`.

---

## [1.3.4] — 2026-04-29

### Naprawiono
- **Reset cause pokazywał `(none)`** — bootloader Optiboot na Arduino Mega 2560 zeruje rejestr MCUSR przed startem aplikacji, ale kopiuje oryginalną wartość do rejestru `r2`. Dodano handler w sekcji `.init0` (uruchamiany PRZED inicjalizacją runtime C++) odczytujący `r2` do zmiennej `.noinit` — diagnostyka teraz pokazuje faktyczną przyczynę resetu zamiast `(none)`.

### Dodano
- **Surowa wartość MCUSR w hex** — na LCD (linia 4) i Serial monitor: `MCUSR=0x04`. Pozwala rozpoznać też flagi nieobsługiwane przez `else if` (np. JTRF — bit 4, JTAG reset).
- Reset cause na Serial monitor — ten sam tekst co na LCD, w formacie: `Reset cause: BROWN-OUT (0x04)`.

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.3.3 → 1.3.4
- `src/main.cpp` — dodanie `g_mcusrMirror` (`.noinit`) + handler `.init0` `getMCUSRFromR2()`; zmiana Serial diagnostyki + dodanie 4. linii LCD z hex MCUSR

---

## [1.3.3] — 2026-04-29

### Dodano
- **Reset cause na LCD** — przyczyna ostatniego resetu (WATCHDOG / BROWN-OUT / EXTERNAL / POWER-ON / none) wyświetlana na ekranie boot przez 3s, w trzeciej linii. Pomocne gdy Serial monitor niedostępny — diagnoza częstych restartów bez podłączenia do PC.

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.3.2 → 1.3.3
- `src/main.cpp` — wypisanie reset cause na LCD po "Boot: ..." (przy istniejącej diagnostyce Serial)

---

## [1.3.2] — 2026-04-28

### Zmieniono
- **`sunTracker.update()` pomijany gdy tracker odłączony** — wywołanie warunkowane DIP bitami `trackerLdrSensorsConnected` (5) i `trackerEnableServoMovement` (6). Gdy któryś z nich jest `false`, `update()` nie jest wywoływane (oszczędność cykli, brak prób ruszania niepodpiętych serw / czytania niepodpiętych LDR-ów).

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.3.1 → 1.3.2
- `src/main.cpp` — guard `if (trackerLdrSensorsConnected && trackerEnableServoMovement)` wokół `sunTracker.update()`

---

## [1.3.1] — 2026-04-28

### Dodano
- **Enkoder zapala backlight** — każda interakcja z enkoderem (obrót w lewo/prawo, klik) zapala podświetlenie LCD na 10s (konfigurowalne).
- Nowy parametr w `config.txt`: `backlight_encoder_ms` (default `10000`).
- Aktualizacja dokumentacji (`README.md`, `WIRING.md`): GPS przeniesiony z opisowo Serial1 (piny 18, 19) na Serial2 (piny 16, 17) — zgodnie z faktycznym kodem. Usunięto stare komentarze "TEST" w `main.cpp`.

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.3.0 → 1.3.1
- `src/ProjectConfig.h` — pole `backlightEncoderDurationMs`
- `src/SDCard.cpp` — parser/writer klucza `backlight_encoder_ms`
- `src/main.cpp` — `enableBacklightFor()` w handlerze enkodera (obrót + klik); poprawione komentarze GPS
- `README.md`, `WIRING.md` — sprostowanie portu GPS

---

## [1.3.0] — 2026-04-28 (devplan005)

### Dodano
- **GPS health detection — 4 stany zamiast 2** (devplan005)
  - `NO_MODULE` (ERROR) — przez 10s nie było ani jednego bajta na Serial2 → moduł niepodpięty / brak zasilania
  - `BAD_DATA` (ERROR) — bajty są, ale przez 15s żadne nie tworzy poprawnego NMEA → uszkodzenie / błędny baudrate / zamienione TX-RX
  - `SEARCHING` (WARNING) — NMEA OK, brak fix
  - `FIXED (n)` (OK) — fix uzyskany
- `GPSModule::getHealth(noDataTimeout, badDataTimeout)` — zwraca aktualny stan modułu
- 2 nowe parametry w `config.txt`:
  - `gps_no_data_timeout_ms` (default 10000)
  - `gps_bad_data_timeout_ms` (default 15000)
- **Diagnostyka przyczyny resetu** w `setup()` — wypisuje na Serial flagi MCUSR (POWER-ON / EXTERNAL / BROWN-OUT / WATCHDOG). Pomocne przy diagnozie restartów (np. brown-out przy podłączonym GPS).

### Naprawiono
- **GPS UART buffer overflow** — `GPSModule::update()` czytał tylko 1 bajt na wywołanie (`if`), co przy 9600 baud i ~960 znaków/s mogło przepełniać 64-bajtowy bufor sprzętowy UART przy dłuższych iteracjach `loop()`. Zmieniono na pętlę `while` — czyta wszystkie dostępne bajty per wywołanie. Podejrzane o przyczynianie się do restartów systemu gdy GPS podpięty.

### Zmieniono
- Wersja firmware: `1.2.0` → `1.3.0`

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.2.0 → 1.3.0
- `src/GPSModule.h` — enum `GPSHealth`, deklaracja `getHealth()`, pola `_firstByteAtMs`/`_lastSentenceAtMs`
- `src/GPSModule.cpp` — pętla `while` w `update()`, tracking timestampów, implementacja `getHealth()`
- `src/ProjectConfig.h` — 2 nowe pola w `Configuration`
- `src/SDCard.cpp` — parser i writer 2 nowych kluczy
- `src/main.cpp` — switch po `getHealth()` w aktualizacji statusu GPS, diagnostyka MCUSR w setup()

---

## [1.2.0] — 2026-04-28 (devplan004)

### Dodano
- **Touch jako manualny włącznik backlight + force sleep** (devplan004)
  - Krótkie tknięcie (<5s) → backlight LCD ON na 30s (konfigurowalne)
  - Długie trzymanie (≥5s) → force sleep (gdy `sleepModeEnabled=true`)
  - Po wybudzeniu (RTC alarm lub touch) → backlight ON na 60s (jak po boot)
  - Mitygacja: po wybudzeniu blokujemy detekcję press do najbliższego release palca
- **3 nowe parametry w `config.txt`:**
  - `backlight_boot_ms` (default 60000) — czas backlight po boot/wake-up
  - `backlight_touch_ms` (default 30000) — czas backlight po krótkim tknięciu
  - `long_press_ms` (default 5000) — próg detekcji long press
- `PowerManager::forceSleep()` — wymusza natychmiastowe `PREPARE_SLEEP`
- `PowerManager::consumeWakeEvent()` — flaga jednorazowa po wybudzeniu

### Zmieniono
- LCD backlight jest teraz **wyłączony domyślnie** w trybie aktywnym (oszczędność energii)
  - Wcześniej: backlight zawsze ON w trybie aktywnym
  - Teraz: ON tylko w oknach (boot 60s, wake-up 60s, touch 30s)
- Touch handling przeniesiony spoza `if (sleepModeEnabled)` — działa też przy wyłączonym sleep mode (tylko backlight 30s, bez force sleep)
- Wersja firmware: `1.1.1` → `1.2.0`

### Pliki zmodyfikowane
- `src/Version.h` — bump 1.1.1 → 1.2.0
- `src/PowerManager.h/cpp` — `forceSleep()`, `consumeWakeEvent()`, flaga `_wakeEventPending`
- `src/ProjectConfig.h` — 3 nowe pola w `Configuration`
- `src/SDCard.cpp` — parser i writer 3 nowych kluczy
- `src/main.cpp` — `g_backlightOffAtMs`, `enableBacklightFor()`, `updateBacklightTimer()`, rewrite touch handling, `g_skipTouchUntilRelease`

---

## [1.1.1] — 2026-04-10 (devplan003)

### Naprawiono
- **BUG1** `main.cpp:122` — Błędne piny I2C w `DATA_PINS_TO_DEENERGIZE`: `A4, A5` → `20, 21` (Mega SDA/SCL)
- **BUG2** `SunTracker.cpp:50` — Usunięto podwójne wywołanie `controlPanel.update()` z `SunTracker::update()`
- **BUG3** `SunTracker.cpp` — Zainicjalizowano `topLeftVal/topRightVal/downLeftVal/downRightVal` na `0` w konstruktorze
- **BUG4** `LcdDisplay.cpp` — Usunięto debug prints (`[DEBUG]`) z `_drawStatusScreen()`
- **BUG5** `PowerManager.cpp:114` — Poprawiono komentarz sleep alarmu + dodano TODO dla konfigurowalnego interwału
- **BUG6** `main.cpp:26` — Poprawiono komentarz pin DHT: `-> 6` → `-> 46`
- **BUG7** `LcdDisplay.cpp:213,218` — Zastąpiono VLA `char buffer[_cols + 1]` → `char buffer[21]` (2 miejsca)

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
