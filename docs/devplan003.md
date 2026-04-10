# devplan003 — Naprawa bugów BUG1–BUG7

| Pole | Wartość |
|------|---------|
| **Nr** | 003 |
| **Data** | 2026-04-10 |
| **Wersja docelowa** | 1.1.1 |
| **Status** | ✅ ZREALIZOWANY |
| **Autor** | Sebastian Josiek |

## Zmiany

| Bug | Plik | Linia | Zmiana |
|-----|------|-------|--------|
| BUG1 | `src/main.cpp` | 122 | `A4, A5` → `20, 21` (I2C piny Mega) |
| BUG2 | `src/SunTracker.cpp` | 50 | Usunąć `controlPanel.update()` z SunTracker::update() |
| BUG3 | `src/SunTracker.cpp` | ~23 | Dodać inicjalizację `topLeftVal(0)` itd. w konstruktorze |
| BUG4 | `src/LcdDisplay.cpp` | ~196 | Usunąć debug prints z `_drawStatusScreen()` |
| BUG5 | `src/PowerManager.cpp` | 114 | Poprawić komentarz "5 minut" → "1 minuta" |
| BUG6 | `src/main.cpp` | 26 | Poprawić komentarz: pin `6` → `46` |
| BUG7 | `src/LcdDisplay.cpp` | 213,232 | VLA `char buffer[_cols + 1]` → `char buffer[21]` |
