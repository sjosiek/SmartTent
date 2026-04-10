# SmartTent — Schemat połączeń

## Moduł MOSFET (zasilanie peryferiów)

Moduł przełącza zasilanie wszystkich peryferiów podczas sleep mode.
Sterowanie: Arduino pin 4 (HIGH = włącz, LOW = wyłącz).

### Wariant A — N-channel MOSFET, low-side switch (częstszy w gotowych modułach)

```
                    ┌─────────────────────┐
                    │   MOSFET MODULE     │
  Arduino pin 4 ───▶│ IN                  │
  Arduino 5V ──────▶│ VCC      OUT+ ──────┼──────┬─── VCC peryferiów
  Arduino GND ─────▶│ GND      OUT- ──────┼──┐   │    (LCD, BME, RTC,
                    └─────────────────────┘  │   │     DHT, GPS, SD)
                                             │   │
  Zasilanie główne (+) ──────────────────────┘   │
  Zasilanie główne (-) ──── GND Arduino ──────────┘
```

**Przepływ prądu:** Zasilanie(+) → OUT+ → peryferia → OUT- → MOSFET → GND

Moduł przełącza masę (GND). Gdy Arduino pin 4 = HIGH → MOSFET przewodzi → obwód zamknięty → peryferia zasilone.

---

### Wariant B — P-channel MOSFET, high-side switch (lepsza izolacja)

```
                    ┌─────────────────────┐
                    │   MOSFET MODULE     │
  Arduino pin 4 ───▶│ IN                  │
  Arduino GND ─────▶│ GND      OUT+ ──────┼──────── VCC peryferiów
  Zasilanie (+) ───▶│ VCC      OUT- ──────┼──────── GND peryferiów
                    └─────────────────────┘
```

**Przepływ prądu:** Zasilanie(+) → VCC modułu → MOSFET → OUT+ → peryferia → OUT- → GND

Moduł przełącza VCC. Gdy Arduino pin 4 = HIGH → wewnętrzny NPN włącza MOSFET → OUT+ aktywny.

---

### Które peryferia są zasilane przez moduł?

Wszystkie peryferia których VCC jest podłączone przez moduł:

| Peryferia | Interfejs |
|-----------|-----------|
| LCD 20x4 | I2C (pin 20, 21) |
| BME280 | I2C (pin 20, 21) |
| RTC DS3231 | I2C (pin 20, 21) — tylko linie danych, VCC RTC może być stałe |
| DHT11/22 | Digital (pin 46) |
| Moduł GPS | Serial1 (pin 18, 19) |
| TM1637 LED | Digital (pin 22, 23) |
| Karta SD | SPI (pin 50–53) |

> **Uwaga:** RTC DS3231 musi mieć stałe zasilanie VCC (podtrzymanie zegara baterią CR2032).
> Przez moduł MOSFET podłączone są tylko linie danych i ewentualnie VCC logiki.

---

## Pełny schemat połączeń Arduino Mega 2560

```
                         Arduino Mega 2560
                    ┌────────────────────────────┐
                    │                            │
   ┌────────────────┤ 20 (SDA) ◄──────────────── ┼── I2C SDA
   │  ┌─────────────┤ 21 (SCL) ◄──────────────── ┼── I2C SCL
   │  │             │                            │
   │  │   ┌─────────┤ 18 (TX1) ──────────────────┼──▶ GPS RX
   │  │   │   ┌─────┤ 19 (RX1) ◄─────────────────┼── GPS TX
   │  │   │   │     │                            │
   │  │   │   │     │  2 (INT0) ◄── RTC SQW (alarm)
   │  │   │   │     │  3 (INT1) ◄── TTP223 OUT (touch)
   │  │   │   │     │  4        ──▶ MOSFET IN
   │  │   │   │     │  9  (PWM) ──▶ Servo H signal
   │  │   │   │     │  10 (PWM) ──▶ Servo V signal
   │  │   │   │     │  13       ──▶ LED builtin (heartbeat)
   │  │   │   │     │  22       ──▶ TM1637 CLK
   │  │   │   │     │  23       ──▶ TM1637 DIO
   │  │   │   │     │  24       ◄── Joystick 1 SW
   │  │   │   │     │  25       ◄── Joystick 2 SW
   │  │   │   │     │  26       ◄── Encoder DT
   │  │   │   │     │  27       ◄── Encoder CLK
   │  │   │   │     │  28       ◄── Encoder SW
   │  │   │   │     │  29       ──▶ Buzzer (+)
   │  │   │   │     │  30       ──▶ 74HC165 LATCH
   │  │   │   │     │  31       ──▶ 74HC165 CLK
   │  │   │   │     │  32       ◄── 74HC165 DATA
   │  │   │   │     │  46       ──▶ DHT11/22 DATA
   │  │   │   │     │  50 MISO  ◄── SD MISO
   │  │   │   │     │  51 MOSI  ──▶ SD MOSI
   │  │   │   │     │  52 SCK   ──▶ SD SCK
   │  │   │   │     │  53 CS    ──▶ SD CS
   │  │   │   │     │                            │
   │  │   │   │     │  A0       ◄── LDR Dół-Lewo
   │  │   │   │     │  A1       ◄── LDR Góra-Lewo
   │  │   │   │     │  A2       ◄── LDR Góra-Prawo
   │  │   │   │     │  A3       ◄── LDR Dół-Prawo
   │  │   │   │     │  A8       ◄── Joystick 1 X
   │  │   │   │     │  A9       ◄── Joystick 1 Y
   │  │   │   │     │  A10      ◄── Joystick 2 X
   │  │   │   │     │  A11      ◄── Joystick 2 Y
   │  │   │   │     │  A12      ◄── Potencjometr 1
   │  │   │   │     │  A13      ◄── Potencjometr 2
   │  │   │   │     │  A14      ◄── Potencjometr 3
   │  │   │   │     │  A15      ◄── Potencjometr 4
   │  │   │   │     └────────────────────────────┘
   │  │   │   │
   │  │   │   └──────────── GPS moduł
   │  │   └──────────────── GPS moduł
   │  │
   │  └───── I2C SCL ──┬── LCD 20x4
   └──────── I2C SDA ──┤── BME280
                       └── RTC DS3231
```

---

## Szczegóły podłączeń — moduł po module

### I2C (magistrala wspólna, pin 20=SDA, 21=SCL)

```
Arduino 20 (SDA) ──┬────────────────────────────────
Arduino 21 (SCL) ──┼────────────────────────────────
                   │
              ┌────┴────┐   ┌─────────┐   ┌──────────┐
              │ LCD 20x4│   │ BME280  │   │DS3231 RTC│
              │ 0x27    │   │ 0x76    │   │          │
              │ VCC─5V  │   │ VCC─3.3V│   │ VCC─3.3V │
              │ GND─GND │   │ GND─GND │   │ GND─GND  │
              └─────────┘   └─────────┘   └──────────┘
```

> Rezystory pull-up I2C (4.7kΩ do 5V) zazwyczaj wbudowane w moduły.

### Moduł GPS (Serial1)

```
Arduino 18 (TX1) ──▶ GPS RX
Arduino 19 (RX1) ◄── GPS TX
Arduino 5V       ──▶ GPS VCC   (przez MOSFET lub bezpośrednio)
Arduino GND      ──▶ GPS GND
```

### Sun Tracker — serwomechanizmy

```
Arduino 9  ──▶ [Servo H] Signal (pomarańczowy/biały)
Arduino 10 ──▶ [Servo V] Signal (pomarańczowy/biały)

Servo VCC (czerwony)  ──▶ 5V (najlepiej z osobnego zasilacza przy dużym obciążeniu)
Servo GND (brązowy/czarny) ──▶ GND (wspólna z Arduino)
```

### Sun Tracker — fotorezystory LDR (dzielnik napięcia)

```
5V ──[LDR]──┬──[R 10kΩ]── GND
            │
           A0 (Dół-Lewo)   / A1 (Góra-Lewo)
           A2 (Góra-Prawo) / A3 (Dół-Prawo)
```

Układ mechaniczny (widok od przodu panelu):
```
  ┌─────────────┐
  │  TL    TR   │    TL = Top-Left  (A1)
  │  [LDR][LDR] │    TR = Top-Right (A2)
  │             │
  │  [LDR][LDR] │    DL = Down-Left  (A0)
  │  DL    DR   │    DR = Down-Right (A3)
  └─────────────┘
```

### RTC DS3231 — alarm

```
DS3231 SQW ──[R 10kΩ pull-up 5V]──▶ Arduino pin 2 (INT0)
DS3231 VCC ──▶ 3.3V (stałe, nie przez MOSFET — podtrzymanie zegara)
DS3231 BAT ──▶ CR2032 (backup battery)
```

### Czujnik dotykowy TTP223

```
TTP223 VCC ──▶ 3.3V lub 5V
TTP223 GND ──▶ GND
TTP223 OUT ──▶ Arduino pin 3 (INT1, RISING)
```

### 74HC165 — czytnik DIP switch (16 bitów)

```
74HC165 SH/LD ──▶ Arduino 30 (LATCH)
74HC165 CLK   ──▶ Arduino 31 (CLK)
74HC165 QH    ──▶ Arduino 32 (DATA)
74HC165 VCC   ──▶ 5V
74HC165 GND   ──▶ GND

Dla 16 bitów: dwa układy 74HC165 w kaskadzie (QH pierwszego → DS drugiego)
```

### Panel sterowania

```
Joystick 1:  VCC──5V  GND──GND  VRx──A8  VRy──A9  SW──24
Joystick 2:  VCC──5V  GND──GND  VRx──A10 VRy──A11 SW──25
Encoder:     VCC──5V  GND──GND  DT──26   CLK──27   SW──28
Buzzer:      (+)──29  (-)──GND
Pot 1–4:     VCC──5V  GND──GND  OUT──A12/A13/A14/A15
```

### TM1637 — wyświetlacz LED 4-cyfrowy

```
TM1637 CLK ──▶ Arduino 22
TM1637 DIO ──▶ Arduino 23
TM1637 VCC ──▶ 5V (przez MOSFET)
TM1637 GND ──▶ GND
```

### Karta SD (SPI)

```
SD MISO ──▶ Arduino 50
SD MOSI ──▶ Arduino 51
SD SCK  ──▶ Arduino 52
SD CS   ──▶ Arduino 53
SD VCC  ──▶ 5V lub 3.3V (zależnie od modułu — zazwyczaj ma regulator)
SD GND  ──▶ GND
```

### DHT11/22

```
DHT VCC  ──▶ 5V (przez MOSFET)
DHT DATA ──[R 10kΩ pull-up 5V]──▶ Arduino 46
DHT GND  ──▶ GND
```

---

## Zasilanie — podsumowanie

```
Zasilanie główne (5V lub 7-12V z regulatorem)
         │
         ├──▶ Arduino VIN / 5V
         │
         └──▶ MOSFET IN (VCC wejście)
                   │
              MOSFET OUT ──▶ VCC peryferiów (LCD, BME, DHT, GPS, TM1637, SD)
                                │
                           sterowanie: Arduino pin 4
                           HIGH = włącz, LOW = wyłącz (sleep mode)

RTC DS3231: zawsze zasilony (VCC stałe + backup CR2032)
Arduino:    zawsze zasilony
Serwomechanizmy: stałe 5V (osobna szyna, duży prąd!)
```
