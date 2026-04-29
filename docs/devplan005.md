# devplan005 — GPS health detection (NO_MODULE / BAD_DATA / SEARCHING / FIXED)

> **Uwaga:** ścieżka pliku w plans/ jest reliktem poprzedniej sesji (plan dla devplan004 — backlight). Aktualna treść dotyczy NOWEGO zadania (devplan005 — GPS health). Plan został wcześniej zaakceptowany przez użytkownika i zapisany w [docs/devplan005.md](c:/PRIV/DEV/SmartTent/docs/devplan005.md).

| | |
|---|---|
| **Wersja docelowa** | 1.3.0 |
| **Data** | 2026-04-28 |
| **Status** | Zatwierdzone — implementacja w toku |

## Context

Obecnie status GPS na ekranie statusów ma tylko 3 stany ([main.cpp:504-514](src/main.cpp#L504-L514)):
- `INITIALIZING` / "INIT..." — domyślny po boot
- `OK` / "FIXED (n)" — gdy `gps.isDataValid()` (czyli `_gps.location.isValid()` z TinyGPS++)
- `WARNING` / "SEARCHING" — gdy nie valid

Problem: **GPS gada przez UART (Serial2)** i Arduino nie ma sposobu wykryć fizycznej obecności modułu — gdy moduł niepodpięty, na linii nie pojawia się ŻADEN bajt, `gps.update()` nigdy nie zwraca `true`, a status zostaje na "SEARCHING" w nieskończoność. User patrząc na ekran nie wie, czy GPS faktycznie szuka satelitów, czy moduł jest po prostu odpięty / uszkodzony / źle podpięte TX-RX.

Hardware: **GY-GPS6MV2 (u-blox NEO-6M)** — wysyła NMEA non-stop @9600 baud niezależnie od fix. Dlatego brak NMEA przez >15s = realny problem (BAD_DATA), brak jakichkolwiek bajtów = niepodpięty (NO_MODULE).

Cel: rozróżnić 4 realne sytuacje:
- **NO_MODULE** (ERROR) — przez 10s nie było żadnego bajta → moduł odpięty / brak zasilania
- **BAD_DATA** (ERROR) — bajty są, ale przez 15s żadne nie tworzy poprawnego NMEA → uszkodzenie / błędny baudrate / zamienione TX-RX
- **SEARCHING** (WARNING) — NMEA OK, brak fix (coldstart, w pomieszczeniu)
- **FIXED (n)** (OK) — fix uzyskany

---

## Decyzje

| Parametr | Default | Konfigurowalny |
|----------|---------|----------------|
| `gps_no_data_timeout_ms` | `10000` | tak (config.txt) |
| `gps_bad_data_timeout_ms` | `15000` | tak (config.txt) |
| Liczba stanów GPSHealth | 4 | nie |
| Bump wersji | 1.2.0 → 1.3.0 (MINOR) | — |

---

## Zmiany — pliki i zakres

### 1. [src/GPSModule.h](src/GPSModule.h) / [src/GPSModule.cpp](src/GPSModule.cpp)

Dodać enum publiczny `GPSHealth { NO_MODULE, BAD_DATA, SEARCHING, FIXED }`.

Nowe pola prywatne:
```cpp
unsigned long _firstByteAtMs = 0;       // 0 = nigdy nie było bajta
unsigned long _lastSentenceAtMs = 0;    // 0 = nigdy nie zdekodowano NMEA
```

**Update `update()` — DWIE zmiany:**

a) Pętla `while` zamiast `if` (krytyczne — fix dla potencjalnego przepełnienia 64B bufora UART przy 9600 baud, podejrzane o powodowanie restartów systemu gdy GPS podpięty):

```cpp
bool GPSModule::update() {
  bool newSentence = false;
  while (_gpsStream.available()) {
    if (_firstByteAtMs == 0) {
      _firstByteAtMs = millis();
    }
    char c = _gpsStream.read();
    if (_gps.encode(c)) {
      _lastSentenceAtMs = millis();
      // ...aktualizacja _isValid, _latitude, etc. (jak dziś)
      newSentence = true;
    }
  }
  return newSentence;
}
```

b) Tracking `_firstByteAtMs` i `_lastSentenceAtMs` jak wyżej.

Nowa metoda `GPSHealth getHealth(unsigned long noDataTimeoutMs, unsigned long badDataTimeoutMs) const;` z logiką:
- Brak `_firstByteAtMs` i `now > noDataTimeout` → NO_MODULE; przed timeoutem → SEARCHING (cold start UI)
- Brak NMEA przez `> badDataTimeout` → BAD_DATA
- `_isValid` → FIXED, inaczej SEARCHING

### 1a. [src/main.cpp:215-220](src/main.cpp#L215-L220) — diagnostyka przyczyny resetu

Odkomentować i rozszerzyć istniejący zaślepiony blok MCUSR — pomoże user'owi zdiagnozować dlaczego system się restartuje przy podłączonym GPS:

```cpp
uint8_t resetFlags = MCUSR;
MCUSR = 0;  // wyczyścić, żeby kolejne resety były wykrywalne
Serial.print(F("Reset cause: "));
if (resetFlags & (1 << PORF))  Serial.print(F("POWER-ON "));
if (resetFlags & (1 << EXTRF)) Serial.print(F("EXTERNAL "));
if (resetFlags & (1 << BORF))  Serial.print(F("BROWN-OUT "));   // <- najpewniejszy podejrzany
if (resetFlags & (1 << WDRF))  Serial.print(F("WATCHDOG "));
Serial.println();
```

**Interpretacja dla user'a:**
- BROWN-OUT = zasilanie nie wyrabia (GPS pobiera +50mA — dodaj kondensator 100µF blisko GPS VCC, lub zasilaj z lepszego źródła)
- WATCHDOG = loop wisi (mało prawdopodobne, WDT jest zakomentowany)
- EXTERNAL = zewnętrzny reset (button/programmer)
- POWER-ON = normalny start

### 2. [src/ProjectConfig.h](src/ProjectConfig.h) — `Configuration`

Dodać:
```cpp
uint32_t gpsNoDataTimeoutMs = 10000;
uint32_t gpsBadDataTimeoutMs = 15000;
```

### 3. [src/SDCard.cpp](src/SDCard.cpp)

Parser (po polach z devplan004): klucze `gps_no_data_timeout_ms`, `gps_bad_data_timeout_ms`. Writer analogicznie z `snprintf` + `configFile.println`.

### 4. [src/main.cpp:504-514](src/main.cpp#L504-L514) — switch po `getHealth()`

Zastąpić obecną binarną logikę na 4-case switch ustawiający `gpsStatus.health` (OK/WARNING/ERROR) i `statusText` ("FIXED (n)"/"SEARCHING"/"BAD DATA"/"NO MODULE"). Bufor 16B w [DeviceStatus.h:18](src/DeviceStatus.h#L18) wystarcza.

### 5. [src/Version.h](src/Version.h) — bump 1.2.0 → 1.3.0

### 6. [CHANGELOG.md](CHANGELOG.md) — wpis 1.3.0 (devplan005)

---

## Reuse istniejących utilities

- `TinyGPSPlus::encode(c)` — już używamy
- `_gpsStream.available()` / `_gpsStream.read()` — już używamy
- `Configuration` + `SDCard::read/writeConfiguration` — wzorzec z devplan004
- `ModuleHealth::ERROR` — istnieje w [DeviceStatus.h](src/DeviceStatus.h)

---

## Side-effects / ryzyka

1. Zła prędkość transmisji → BAD_DATA. OK.
2. Coldstart na zewnątrz → NMEA leci, `location.isValid() == false` → SEARCHING. OK.
3. `millis()` wraparound — różnice bezpieczne dla <49 dni.
4. Fizyczne odłączenie po boot — `_firstByteAtMs != 0` zostaje, ale BAD_DATA wystarczy do diagnostyki.
5. Wolny moduł (NMEA raz na 1s) — `bad_data_timeout=15000` daje 15× margines.

---

## Verification

1. **NO_MODULE**: odepnij GPS → po 10s status "NO MODULE" (ERROR).
2. **BAD_DATA**: `Serial2.begin(4800)` lub zamień TX-RX → po 15s "BAD DATA".
3. **SEARCHING**: GPS w pomieszczeniu → "SEARCHING".
4. **FIXED**: GPS na zewnątrz (LED PPS mruga 1Hz) → "FIXED (n)".
5. **Konfigurowalność**: `gps_no_data_timeout_ms=5000` w config.txt → NO_MODULE po 5s.
6. **Kompilacja**: `pio run` bez warningów.

---

## Nie wchodzi w scope

- Auto-detect baudrate (mało prawdopodobny use case dla NEO-6M @9600)
- Hardware reset GPS przy NO_MODULE (brak pinu reset)
- Zmiana na ISR dla UART (polling z `while` w update() wystarcza, ISR dodawałby konflikt z innymi przerwaniami)
- Detekcja utraty fix po jego uzyskaniu (już dziś działa przez `isDataValid()`)
