# devplan002 — Scalenie origin/develop → main

| Pole | Wartość |
|------|---------|
| **Nr** | 002 |
| **Data** | 2026-04-10 |
| **Wersja docelowa** | 1.1.0 |
| **Status** | ✅ ZREALIZOWANY |
| **Autor** | Sebastian Josiek |

## Powód

Branch `develop` jest source of truth i zawiera 2 commity których nie ma na `main`:
- `c622f76` — more wdt_reset()
- `cbeb2a8` — Poprawki dla GPS

## Zmiany z develop

| Obszar | Zmiana |
|--------|--------|
| `GPSModule.h/.cpp` | Migracja NeoGPS → TinyGPS++ (kompletny rewrite parsera) |
| `platformio.ini` | Dodana lib `mikalhart/TinyGPSPlus` |
| `src/main.cpp` | GPS na Serial2, WDT wyłączony (debug), GPS status fix |

## Konflikty do rozwiązania

Tylko `src/main.cpp` — obie strony go modyfikowały:
- **develop:** Serial2, WDT zakomentowany, GPS status fix
- **main (nasze):** `#include "Version.h"`, boot messages z FIRMWARE_BUILD_INFO

Strategia: zachowaj wszystko z develop + nasze dodatki (Version.h, FIRMWARE_BUILD_INFO)
