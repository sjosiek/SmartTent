---
name: SmartTent — analiza kodu i znalezione bugi
description: Szczegółowa analiza main.cpp i wszystkich modułów, znalezione bugi z lokalizacją
type: project
---

## Znalezione bugi (od krytycznych do kosmetycznych)

### BUG 1 — KRYTYCZNY: Błędne piny I2C w liście de-energetyzacji
**Plik:** `main.cpp` linia 121
**Kod:**
```cpp
const uint8_t DATA_PINS_TO_DEENERGIZE[] = {
    A4, A5,  // ← BŁĄD! To piny I2C dla Arduino Uno/Nano, nie Mega!
```
**Problem:** Na Arduino Mega 2560 magistrala I2C jest na pinach **20 (SDA) i 21 (SCL)**. Piny A4/A5 to zwykłe wejścia analogowe. W efekcie linie I2C (LCD, BME280, RTC) NIE są de-energetyzowane przed snem → phantom power draw.
**Naprawa:** Zmienić `A4, A5` na `20, 21`.
**Status:** Aktualnie nieaktywny — sleep mode wyłączony (DIP bit 0 = 0 w symulacji main.cpp:234). Ujawni się gdy sleep zostanie włączony.

---

### BUG 2 — REAL: Podwójne wywołanie `controlPanel.update()`
**Pliki:** `main.cpp` linia 493 + `SunTracker.cpp` linia 50
**Problem:** Gdy `useJoystick=true`, `SunTracker::update()` wywołuje `controlPanel.update()` wewnętrznie, ale `main.cpp::loop()` też go wywołuje. Metoda `DebouncedButton::wasPressed()` zwraca true tylko raz (stan jest konsumowany przy odczycie).
**Naprawa:** Usunąć `controlPanel.update()` z `SunTracker::update()` – panel jest własnością main.cpp.
**Status:** Działa przypadkowo — SunTracker wywołuje update() i od razu sprawdza wasJoy1Clicked(), więc stan jest świeży. Code smell, nie realny bug w obecnej architekturze.

---

### BUG 3 — REAL: Nieinicjalizowane zmienne członkowskie w SunTracker
**Plik:** `SunTracker.cpp` konstruktor
**Zmienne:** `topLeftVal`, `topRightVal`, `downLeftVal`, `downRightVal`
**Problem:** Nie są inicjalizowane w konstruktorze. Przed pierwszym wywołaniem `handleTrackingLogic()` (stan RUNNING), metoda `getLdrValues()` zwróci garbage values → ekran TRACKER pokazuje śmieci przez pierwsze minuty działania.
**Naprawa:** Dodać do listy inicjalizacyjnej konstruktora:
```cpp
topLeftVal(0), topRightVal(0), downLeftVal(0), downRightVal(0)
```

---

### BUG 4 — UCIĄŻLIWY: Debug prints w `_drawStatusScreen()` bez guarda
**Plik:** `LcdDisplay.cpp` linie 194–220
**Problem:** `Serial.println(F("\n[DEBUG] Rysowanie ekranu statusu..."))` i kilka kolejnych linii wywoływane przy każdym renderowaniu ekranu STATUS (co 5 sekund). Zaśmiecają serial monitor.
**Naprawa:** Usunąć lub otoczyć `#ifdef DEBUG_LCD`.

---

### BUG 5 — KOMENTARZ: Sleep alarm hardcoded 1 minuta
**Plik:** `PowerManager.cpp` linia 114
**Kod:**
```cpp
DateTime future(now + TimeSpan(0, 0, 1, 0));  // zawsze 1 minuta
```
Czas wybudzenia ze snu jest hardcoded (1 min), nie pochodzi z `active_mode_minutes` z config.txt. Komentarz wyżej mówi "5 minut" — stara kopia.
**Naprawa:** Rozważyć nowy klucz `sleep_interval_minutes` w config.txt.

---

### BUG 6 — KOMENTARZ: Zły pin DHT w komentarzu main.cpp
**Plik:** `main.cpp` linia 27
Komentarz mówi `DHT -> 6`, rzeczywisty pin: `DHT_PIN = 46` (z `ProjectConfig.h`). Kod używa stałej, działa poprawnie.

---

### BUG 7 — MINOR: VLA w `_drawStatusScreen()`
**Plik:** `LcdDisplay.cpp` linia 211
```cpp
char buffer[_cols + 1];  // VLA — niestandardowe w C++
```
Bezpieczniej: `char buffer[21]` (LCD ma zawsze 20 kolumn).

---

## Obserwacje architektoniczne

### Dobre praktyki zastosowane w kodzie
- WDT (watchdog timer) z `wdt_reset()` w każdej pętli
- `F()` makro dla string literals w PROGMEM — oszczędność RAM
- `snprintf` zamiast `sprintf` — bezpieczne buforowanie
- `sscanf` do parsowania komend serial — bez alokacji
- `checkAndInit()` w LcdDisplay — auto-odtwarzanie po zaniku I2C
- BME280 auto-reinit przy błędzie I2C

### Potencjalne ulepszenia (nie bugi)
- `SLEEP_MODE_IDLE` zamiast `PWR_DOWN` — wymuszony przez RISING interrupt TTP223. Intentional.
- `gps.begin()` nie wywoływane w setup() — ale metoda jest pusta, brak wpływu
- `printStatusReport()` zakomentowany w loop() — debug kod do usunięcia
- SunTracker kalibracja przez Serial blokuje UI (czeka na znak z portu)
