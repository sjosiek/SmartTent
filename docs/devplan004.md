# Plan: Backlight off-by-default + touch jako manualny włącznik / force sleep

## Context

Obecnie podświetlenie LCD jest **zawsze włączone** w trybie aktywnym — `LcdDisplay::checkAndInit()` wywołuje `lcd.backlight()` automatycznie ([LcdDisplay.cpp:22](src/LcdDisplay.cpp#L22)) i nikt go potem nie gasi (poza `PowerManager::prepareToSleep()` przed snem, [PowerManager.cpp:105](src/PowerManager.cpp#L105)). Touch sensor (pin 3, INT1) ma dziś dwie role: wybudza ze snu (ISR `RISING`) i resetuje timer aktywności w trybie aktywnym ([main.cpp:399-404](src/main.cpp#L399-L404)).

Zmiana: backlight ma być **wyłączony domyślnie** (oszczędność energii — projekt jest zasilany solar/bateria z power management). Wyjątki:
- Okno **boot + 60s po końcu setup()** — żeby zobaczyć ekran statusów modułów i pierwszy ekran główny
- Okno **60s po wybudzeniu ze snu** (RTC alarm lub touch) — analogicznie do boot
- **30s po krótkim tknięciu** (<5s) w trybie aktywnym
- Dodatkowo: **długie trzymanie (≥5s)** = force sleep (gdy `sleepModeEnabled=true`)

## Decyzje doprecyzowane z użytkownikiem

| Aspekt | Wartość |
|--------|---------|
| Start okna 60s po boot | Od końca `setup()` (po `lcd.showMainScreen()`) |
| Detekcja long press | Natychmiast po 5s trzymania (nawet jeśli user trzyma dalej) |
| Backlight po wake-up | 60s (jak boot) |
| Touch przy `sleepModeEnabled=false` + long press | No-op (sleep wyłączony) |

---

## Zmiany — pliki i zakres

### 1. [src/PowerManager.h](src/PowerManager.h) / [src/PowerManager.cpp](src/PowerManager.cpp)

Dodać dwie publiczne metody:

```cpp
void forceSleep();          // przełącza _state na PREPARE_SLEEP natychmiast
bool consumeWakeEvent();    // zwraca true raz po obudzeniu (flag-based)
```

- `forceSleep()` — body: `_state = SystemState::PREPARE_SLEEP;` (analogicznie do timera w `update()` linia 56-61).
- `consumeWakeEvent()` — flaga `_wakeEventPending` ustawiana w `handleWakeUp()` (PowerManager.cpp:146-158), kasowana przy odczycie. Pozwala main.cpp dowiedzieć się, że właśnie nastąpiło wybudzenie, i włączyć backlight na 60s.

**NIE** wywoływać `_lcd->backlight()` wewnątrz PowerManager — backlight ma być sterowany centralnie w main.cpp (single source of truth).

### 2. [src/main.cpp](src/main.cpp) — globalny stan backlight

Nowe zmienne globalne (obok `g_runtimeFlags`):

```cpp
static unsigned long g_backlightOffAtMs = 0;   // 0 = OFF, >0 = wyłącz przy millis() >= tej wartości
```

Nowe funkcje pomocnicze (statyczne, plik):

```cpp
void enableBacklightFor(unsigned long durationMs) {
  lcd.backlight();
  g_backlightOffAtMs = millis() + durationMs;
}

void updateBacklightTimer() {
  if (g_backlightOffAtMs != 0 && (long)(millis() - g_backlightOffAtMs) >= 0) {
    lcd.noBacklight();
    g_backlightOffAtMs = 0;
  }
}
```

Uwaga: cast `(long)` zabezpiecza przed wraparoundem `millis()` po ~49 dniach.

### 3. [src/main.cpp](src/main.cpp) — `setup()` (koniec, ok. linia 389)

Po `lcd.showMainScreen()`:

```cpp
enableBacklightFor(60000UL);  // backlight ON jeszcze 60s po boot
```

`lcd.init()` na linii 266 nadal włącza backlight (przez `checkAndInit`), więc cała sekwencja statusów modułów (delay 3s + 3s + 3s + welcome 3s) jest podświetlona — bez zmian.

### 4. [src/main.cpp](src/main.cpp) — `loop()` / `handleActiveMode()`

#### a) Wykrycie wake-up (z PowerManager) — w `loop()` lub na początku `handleActiveMode()`:

```cpp
if (powerManager.consumeWakeEvent()) {
  enableBacklightFor(60000UL);  // 60s po wybudzeniu
}
```

#### b) Touch handling — przepisać blok [main.cpp:399-404](src/main.cpp#L399-L404):

```cpp
static unsigned long touchPressStartMs = 0;
static bool longPressFired = false;

touchSensor.update();

if (touchSensor.isPressed()) {
  if (touchPressStartMs == 0) {
    touchPressStartMs = millis();
    longPressFired = false;
  } else if (!longPressFired && (millis() - touchPressStartMs >= 5000UL)) {
    longPressFired = true;
    if (g_runtimeFlags.sleepModeEnabled) {
      powerManager.forceSleep();
    }
    // gdy sleepMode=false → no-op
  }
} else {
  // release
  if (touchPressStartMs != 0 && !longPressFired) {
    // krótkie tknięcie
    enableBacklightFor(30000UL);
    if (g_runtimeFlags.sleepModeEnabled) {
      powerManager.resetActiveTimer();
    }
  }
  touchPressStartMs = 0;
  longPressFired = false;
}
```

**Zmiana scope:** touch handling wyjmujemy spoza `if (g_runtimeFlags.sleepModeEnabled)` — backlight 30s po krótkim tknięciu działa **niezależnie** od flagi sleep mode. Force sleep i `resetActiveTimer` warunkowane flagą wewnątrz.

#### c) `updateBacklightTimer()` — w `loop()`, najlepiej na początku każdej iteracji (zaraz po `wdt_reset()` lub przed `LcdDisplay.update()`).

### 5. [src/LcdDisplay.cpp](src/LcdDisplay.cpp)

**BEZ ZMIAN.** `lcd.backlight()` w `checkAndInit()` zostaje — to jest pożądane na boot. Wyłączanie zostanie wykonane przez `g_backlightOffAtMs` po `enableBacklightFor(60000)` na końcu setup().

### 6. [src/PowerManager.cpp:105](src/PowerManager.cpp#L105)

Linia `_lcd->noBacklight()` w `prepareToSleep()` — **zostawić** (nadal ma sens przed snem, choć `g_backlightOffAtMs` już mógł zgasić; wywołanie idempotentne).

---

## Reuse istniejących utilities

- [DebouncedButton::isPressed()](src/DebouncedButton.cpp#L47) — używamy do śledzenia trzymania; nie ma potrzeby rozszerzać klasy ani zaśmiecać API używanego też w ControlPanel.
- [PowerManager::resetActiveTimer()](src/PowerManager.h) — istnieje, użyjemy bez zmian na krótkim tknięciu.
- [LcdDisplay::backlight() / noBacklight()](src/LcdDisplay.cpp#L329-L335) — istnieją wrappery, użyjemy bez zmian.
- `g_runtimeFlags.sleepModeEnabled` — flaga DIP switch z [ProjectConfig.h:31](src/ProjectConfig.h#L31), warunkujemy nią force sleep i resetActiveTimer.

---

## Side-effects / ryzyka

1. **`millis()` wraparound** — używamy castu `(long)` w `updateBacklightTimer()`, więc OK. Long press timer (`millis() - touchPressStartMs`) też bezpieczny dla różnic <49 dni.
2. **Touch ISR po wybudzeniu** — pierwszy touch wybudzający ze snu ustawia `g_wakeUpSource = MANUAL_TOUCH` w ISR. Po `handleWakeUp()` palec może wciąż leżeć na sensorze → `isPressed()` zwróci true → start trackingu long press. **Ryzyko:** jeśli user trzyma palec przez 5s podczas wybudzania, system od razu wraca do snu. Mitygacja: po `consumeWakeEvent()` można ustawić `touchPressStartMs = 0` i `longPressFired = true` (skip pierwszego cyklu trzymania). **Wpisuję do planu jako warunek implementacji.**
3. **Boot + force sleep** — gdyby user trzymał touch >5s tuż po boot przy `sleepModeEnabled=true`, system od razu pójdzie spać. To poprawne zachowanie wg specyfikacji.
4. **`sleepModeEnabled=false`**: long press to no-op. Wcześniej user potwierdził.

---

## Verification (manualnie, na hardware)

1. **Boot** — po wgraniu firmware: LCD świeci podczas inicjalizacji modułów (statusy modułów, welcome). Po `lcd.showMainScreen()` świeci jeszcze 60s, potem gaśnie. ✅ jeśli backlight gaśnie ~76s od włączenia zasilania.
2. **Krótkie tknięcie (<5s)** — w trybie aktywnym z wygaszonym ekranem: tknij → backlight ON, po 30s OFF. Powtórzenie tknięcia w ciągu 30s przedłuża okno do 30s od ostatniego tknięcia. ✅
3. **Długie trzymanie (≥5s) przy `sleepModeEnabled=true` (DIP bit 0 = 1)**: trzymaj 5s → system natychmiast przechodzi w sleep ("Stan: PREPARE_SLEEP" na Serial), backlight OFF, MOSFET OFF. ✅
4. **Wake-up przez RTC alarm**: po wybudzeniu LCD świeci 60s, potem OFF. ✅
5. **Wake-up przez touch**: tknij gdy system w sleep → wybudzenie → LCD świeci 60s. **Test brzegowy:** trzymaj palec >5s podczas wybudzania → nie powinno od razu wracać do snu (mitygacja z punktu "Side-effects 2"). ✅
6. **`sleepModeEnabled=false` (DIP bit 0 = 0)**: krótkie tknięcie → backlight 30s. Długie trzymanie → no-op (system nie idzie spać). ✅
7. **Kompilacja PlatformIO**: `pio run` bez warningów / błędów na `[env:megaatmega2560]`.
8. **Wersjonowanie + changelog** — zgodnie z workflow `feedback_workflow.md`: bump wersji w `Version.h` (1.1.1 → 1.2.0, nowa funkcjonalność = MINOR), wpis w `CHANGELOG.md`, ten plik jako `docs/devplan004.md` po finalnej akceptacji.

---

## Konfiguracja przez config.txt (nowe parametry)

Trzy nowe wartości czasowe trafiają do `config.txt` (per request użytkownika), żeby można było je tunować bez rekompilacji.

### a) [src/ProjectConfig.h](src/ProjectConfig.h) — rozszerzenie `struct Configuration`

```cpp
struct Configuration {
  uint32_t activeModeMinutes = 5;
  uint32_t sensorUpdateIntervalMs = 1000;
  uint32_t trackerUpdateIntervalMs = 300000;
  uint8_t  ledBrightness = 7;
  // NOWE:
  uint32_t backlightBootDurationMs = 60000;   // 60s po boot / po wake-up
  uint32_t backlightTouchDurationMs = 30000;  // 30s po krótkim tknięciu
  uint32_t longPressThresholdMs = 5000;       // próg detekcji long press
};
```

### b) [src/SDCard.cpp:100-103](src/SDCard.cpp#L100-L103) — `readConfiguration()`

Dodać 3 linie parsowania:
```cpp
if (strcmp(key, "backlight_boot_ms") == 0)  config.backlightBootDurationMs = atol(value);
if (strcmp(key, "backlight_touch_ms") == 0) config.backlightTouchDurationMs = atol(value);
if (strcmp(key, "long_press_ms") == 0)      config.longPressThresholdMs = atol(value);
```

### c) [src/SDCard.cpp:129-132](src/SDCard.cpp#L129-L132) — `writeConfiguration()`

Dodać 3 wpisy zapisu (po `led_brightness`):
```cpp
snprintf(buffer, sizeof(buffer), "backlight_boot_ms=%lu",  config.backlightBootDurationMs);  configFile.println(buffer);
snprintf(buffer, sizeof(buffer), "backlight_touch_ms=%lu", config.backlightTouchDurationMs); configFile.println(buffer);
snprintf(buffer, sizeof(buffer), "long_press_ms=%lu",      config.longPressThresholdMs);     configFile.println(buffer);
```

### d) Użycie w `main.cpp`

Zamiast hardcode `60000UL` / `30000UL` / `5000UL`:
- `enableBacklightFor(g_config.backlightBootDurationMs)` (boot, wake-up)
- `enableBacklightFor(g_config.backlightTouchDurationMs)` (krótkie tknięcie)
- `if (millis() - touchPressStartMs >= g_config.longPressThresholdMs)` (long press)

Jeśli `config.txt` nie istnieje na SD lub klucze nieobecne — działają defaulty z `Configuration` struct (60000/30000/5000), więc backwards-compatible z istniejącymi kartami SD.

---

## Nie wchodzi w scope

- Modyfikacja `DebouncedButton` (long-press detection robimy inline w main.cpp).
- Zmiana ISR `wakeUpISR_Touch` (nadal `RISING`, bez różnicowania short/long).
- Zmiana `LcdDisplay::checkAndInit()` (`lcd.backlight()` zostaje na boot).
