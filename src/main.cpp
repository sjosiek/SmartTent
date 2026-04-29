// Plik główny: main.cpp
// Wersja z poprawioną logiką w funkcji setup()

/*
 * ==========================================================================
 * --- MAPA POŁĄCZEŃ (Arduino Mega 2560) ---
 * ==========================================================================
 *
 * --- Magistrala I2C (SDA: 20, SCL: 21) ---
 *   - Wyświetlacz LCD 20x4 (SDA -> 20, SCL -> 21)
 *   - Czujnik BME280 (SDA -> 20, SCL -> 21)
 *   - Zegar RTC DS3231 (SDA -> 20, SCL -> 21)
 *
 * --- Magistrala SPI (MOSI: 51, MISO: 50, SCK: 52) ---
 *   - Czytnik kart SD (CS -> 53)
 *
 * --- Magistrala Serial2 (RX2: 17, TX2: 16) ---
 *   - Moduł GPS (GPS TX -> 17, GPS RX -> 16)
 *
 * --- Zasilanie i Przerwania ---
 *   - Moduł zasilania (MOSFET Gate) -> 4
 *   - Zegar RTC DS3231 (SQW / INT) -> 2 (Przerwanie 0)
 *   - Czujnik dotykowy TTP223 (OUT) -> 3 (Przerwanie 1)
 *
 * --- Czujniki ---
 *   - Czujnik DHT11/22 (DATA) -> 46
 *
 * --- Wyświetlacze ---
 *   - Wyświetlacz LED TM1637 (CLK -> 22, DIO -> 23)
 *
 * --- Sun Tracker ---
 *   - Serwo poziome (Signal) -> 9
 *   - Serwo pionowe (Signal) -> 10
 *   - Fotorezystor (Góra-Lewo) -> A1
 *   - Fotorezystor (Góra-Prawo) -> A2
 *   - Fotorezystor (Dół-Lewo) -> A0
 *   - Fotorezystor (Dół-Prawo) -> A3
 *
 * --- Panel Sterowania (Control Panel) ---
 *   - Joystick 1 (X -> A8, Y -> A9, SW -> 24)
 *   - Joystick 2 (X -> A10, Y -> A11, SW -> 25)
 *   - Enkoder obrotowy (DT -> 26, CLK -> 27, SW -> 28)
 *   - Buzzer (+) -> 29
 */

#include <SPI.h>
#include <SD.h>
#include <Arduino.h>
#include "ProjectConfig.h"   // Dołączamy centralną konfigurację projektu
#include "WeatherSensor.h"
#include "Clock.h"
#include "LcdDisplay.h"
#include "LedDisplay.h"
#include "Timer.h"
#include "DhtSensor.h"
#include "PowerManager.h"
#include "CommandHandler.h"
#include "DebouncedButton.h" // Dołączamy nową klasę
#include "SDCard.h"          // Dołączamy nową klasę
#include "SmoothServo.h"     // Dołączamy klasę serwomechanizmu
#include "SensorData.h"      // Dołączamy strukturę danych
#include "SunTracker.h"      // SUnTracker
#include "ControlPanel.h"    // Dołączamy klasę panelu sterowania
#include "GPSModule.h"    // GPS
#include "HardwareConfigReader.h" // Czytnik DIP switch
#include "SoundPlayer.h"     // ZMIANA: Dołączamy nową klasę do obsługi dźwięków
#include "DeviceStatus.h"    // ZMIANA: Dołączamy nową definicję statusu
#include "Version.h"         // Wersja firmware
// #include <avr/wdt.h>         // ZMIANA: Dołączamy bibliotekę Watchdog Timera (wyłączony podczas debugowania GPS)

// --- Konfiguracja działania trackera---
// Zmieniono na standardową inicjalizację C++, aby zapewnić kompatybilność z kompilatorem avr-gcc.
const SunTrackerPins trackerPins = {
    HORIZONTAL_SERVO_PIN,
    VERTICAL_SERVO_PIN,
    LDR_TOP_LEFT_PIN,
    LDR_TOP_RIGHT_PIN,
    LDR_DOWN_LEFT_PIN,
    LDR_DOWN_RIGHT_PIN,
};

const SunTrackerConfig trackerConfig = {
    0,    // servoVMinAngle
    90,   // servoVMaxAngle
    0,    // servoHMinAngle
    180,  // servoHMaxAngle
    false, // performLdrCalibration - zostanie ustawione z DIP
    false, // performServoCalibration - zostanie ustawione z DIP
    false, // performInitialSearch - zostanie ustawione z DIP
    false, // useJoystick - zostanie ustawione z DIP
    false, // usePotentiometers (nieużywane, ale musi być w inicjalizatorze)
    false, // ldrSensorsConnected - zostanie ustawione z DIP
    false, // enableServoMovement - zostanie ustawione z DIP
    false, // enableDebugPrint - zostanie ustawione z DIP
    100,  // defaultServoSpeed
    50,  // defaultTolerance
    300000 // Domyślna wartość, która zostanie nadpisana z konfiguracji
};


// --- Centralna Konfiguracja Serwomechanizmów ---
struct ServoConfig {
  uint8_t pin;
  const char* name;
};

// const ServoConfig servoConfigs[] = {
//   { SERVO1_PIN, "Wywietrznik" },
//   { SERVO2_PIN, "Klapa" }
//   // Możesz tu dodać więcej serw, np. { A2, "Drzwi" }
// };

const ServoConfig servoConfigs[] = {};

const int SERVO_COUNT = sizeof(servoConfigs) / sizeof(servoConfigs[0]);
// Definicja timeoutu dla magistrali I2C w mikrosekundach.
// Zapobiega to zawieszeniu się programu, gdy urządzenie I2C nagle straci zasilanie.
const uint32_t I2C_TIMEOUT_US = 25000; // 25000 mikrosekund = 25 milisekund

// Tablica pinów danych, które muszą być de-energetyzowane przed uśpieniem
const uint8_t DATA_PINS_TO_DEENERGIZE[] = {
    20, 21,             // I2C: SDA, SCL (Mega: pin 20=SDA, 21=SCL)
    DHT_PIN,            // DHT11
    LED_CLK_PIN,        // LED Display
    LED_DIO_PIN,        // LED Display,
    MISO, MOSI, SCK, SD_CS_PIN, // SPI dla karty SD (używamy standardowych stałych Arduino)
    // SERVO1_PIN, SERVO2_PIN // Piny serwomechanizmów
    // UWAGA: Jeśli dodasz serwa, pamiętaj o dodaniu ich pinów tutaj!
};
const uint8_t DATA_PINS_COUNT = sizeof(DATA_PINS_TO_DEENERGIZE) / sizeof(DATA_PINS_TO_DEENERGIZE[0]);

// --- Konfiguracja i inicjalizacja Panelu Sterowania ---
// Musi być zdefiniowany PRZED SunTrackerem, ponieważ jest do niego przekazywany.
// Zmieniono na standardową inicjalizację C++, aby zapewnić kompatybilność z kompilatorem avr-gcc.
const ModulePins controlPanelPins = {
  JOY1_X_PIN, JOY1_Y_PIN, JOY1_SW_PIN,
  JOY2_X_PIN, JOY2_Y_PIN, JOY2_SW_PIN,
  ENC_DT_PIN, ENC_CLK_PIN, ENC_SW_PIN,
  BUZZER_PIN,
  POT1_PIN, POT2_PIN, POT3_PIN, POT4_PIN // ZMIANA: Przekazujemy piny potencjometrów
};

ControlPanel controlPanel(controlPanelPins);

// ZMIANA: Inicjalizacja odtwarzacza dźwięków
SoundPlayer soundPlayer(controlPanel);

//Inicjalizacja modułów
SunTracker sunTracker(trackerPins, trackerConfig, controlPanel);  //SUnTracker

Clock clock;                   // RTC
DhtSensor dhtSensor(DHT_PIN, DHT_TYPE);          // DHT11
WeatherSensor sensor;          //BME 280

LcdDisplay lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);  // LCD
LedDisplay led(LED_CLK_PIN, LED_DIO_PIN);         // LED

// Moduł GPS na porcie Serial2 (TX2 = pin 16, RX2 = pin 17).
GPSModule gps(Serial2);

// ZMIANA: Inicjalizacja czytnika przełączników DIP
HardwareConfigReader dipReader(DIP_LATCH_PIN, DIP_CLOCK_PIN, DIP_DATA_PIN);

// Czujnik dotykowy jest aktywny stanem wysokim (nie ma zworek do zmiany logiki).
DebouncedButton touchSensor(TOUCH_SENSOR_PIN, ActiveState::ACTIVE_HIGH);

PowerManager powerManager(POWER_CONTROL_PIN, RTC_ALARM_PIN, TOUCH_SENSOR_PIN, DATA_PINS_TO_DEENERGIZE, DATA_PINS_COUNT);
SDCard sdCard(SD_CS_PIN);
// Deklarujemy tablicę serw. Używamy "max(1, SERVO_COUNT)", aby uniknąć
// niestandardowej tablicy o zerowej długości, gdy serwa są wyłączone.
// Ten dodatkowy element nigdy nie będzie użyty, ponieważ pętle są chronione przez SERVO_COUNT.
SmoothServo servos[max(1, SERVO_COUNT)];

 
SensorData g_sensorData; // Zastępujemy wiele zmiennych globalnych jedną strukturą
Configuration g_config;  // Globalny obiekt przechowujący konfigurację
RuntimeFlags g_runtimeFlags; // Globalny obiekt flag odczytanych z DIP
 
CommandHandler commandHandler(clock, lcd, sdCard, g_config, servos, SERVO_COUNT); // Przekazujemy tablicę do CommandHandler
 
 
Timer sensorUpdateTimer(1000); // Domyślny interwał, zostanie nadpisany przez konfigurację z SD
Timer ledUpdateTimer(500);
Timer heartbeatTimer(1000);  // ZMIANA: Heartbeat co 1 sekundę
Timer builtinLedTimer(1000); // Timer do mrugania wbudowaną diodą LED
Timer errorLedTimer(200);    // Szybszy timer do sygnalizacji błędu

// --- ZMIANA: Globalna tablica statusów modułów ---
enum class ModuleID {
    RTC, BME280, DHT11, SD_CARD, GPS,
    CMD_HANDLER, EXTRA_SERVOS, CONTROL_PANEL,
    MODULE_COUNT
};

ModuleStatus g_moduleStatuses[static_cast<int>(ModuleID::MODULE_COUNT)] = {
    { "Zegar RTC",    ModuleHealth::UNKNOWN, "..." },
    { "BME280",       ModuleHealth::UNKNOWN, "..." },
    { "DHT11",        ModuleHealth::UNKNOWN, "..." },
    { "Karta SD",     ModuleHealth::UNKNOWN, "..." },
    { "GPS",          ModuleHealth::UNKNOWN, "..." },
    { "Cmd Handler",  ModuleHealth::OK,      "OK" },
    { "Extra Servos", ModuleHealth::UNKNOWN, "..." },
    { "Control Panel",ModuleHealth::UNKNOWN, "..." }
};
// ----------------------------------------------------

// --- MCUSR mirror via Optiboot r2 (devplan: diagnostyka twardych resetów 1.3.4) ---
// Bootloader Optiboot zeruje MCUSR przed startem aplikacji, ale kopiuje
// oryginalną wartość do rejestru r2. Odczytujemy r2 w sekcji .init0
// (uruchamiane PRZED inicjalizacją runtime C++) i zapisujemy do .noinit
// (zmienna nie jest zerowana przez startup code). Następnie setup() czyta
// to bezpośrednio.
uint8_t g_mcusrMirror __attribute__((section(".noinit")));

void getMCUSRFromR2(void) __attribute__((naked, used, section(".init0")));
void getMCUSRFromR2(void) {
  __asm__ __volatile__ ("mov %0, r2\n" : "=r" (g_mcusrMirror));
}

// --- Backlight scheduling ---
// 0 = backlight wygaszony / brak aktywnego okna; >0 = wyłącz przy millis() >= wartość.
static unsigned long g_backlightOffAtMs = 0;

// Po wybudzeniu ze snu palec może wciąż leżeć na sensorze. Bez tej flagi pierwszy
// 5s trzymania zaraz po wake-up od razu by zafire'ował force sleep.
// Flaga blokuje cały touch-handling do najbliższego release.
static bool g_skipTouchUntilRelease = false;

static void enableBacklightFor(unsigned long durationMs) {
  lcd.backlight();
  g_backlightOffAtMs = millis() + durationMs;
  if (g_backlightOffAtMs == 0) g_backlightOffAtMs = 1; // 0 ma znaczenie sentinel
}

static void updateBacklightTimer() {
  if (g_backlightOffAtMs != 0 && (long)(millis() - g_backlightOffAtMs) >= 0) {
    lcd.noBacklight();
    g_backlightOffAtMs = 0;
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Inicjalizacja wbudowanej diody LED
  Serial.begin(9600);
  // Inicjalizacja portu szeregowego dla GPS — Serial2 @ 9600 baud.
  Serial2.begin(9600);

  // Diagnostyka przyczyny ostatniego resetu. Czytamy g_mcusrMirror, a nie MCUSR.
  // Bootloader Optiboot zeruje MCUSR ale kopiuje do r2 (my czytamy r2 w .init0).
  // Bootloader stk500v2 (default na Mega 2560) NIE kopiuje do r2 — wtedy g_mcusrMirror
  // to losowy SRAM. Wykrywamy to po ustawionych bitach 5/6/7 (zarezerwowane = 0
  // w realnym MCUSR) i oznaczamy jako "(invalid)".
  uint8_t resetFlags = g_mcusrMirror;
  bool resetFlagsReliable = ((resetFlags & 0xE0) == 0);

  Serial.print(F("Reset cause: "));
  if (!resetFlagsReliable) {
    Serial.print(F("(invalid - bootloader)"));
  } else if (resetFlags & (1 << WDRF))  Serial.print(F("WATCHDOG"));
  else if (resetFlags & (1 << BORF))    Serial.print(F("BROWN-OUT"));
  else if (resetFlags & (1 << EXTRF))   Serial.print(F("EXTERNAL"));
  else if (resetFlags & (1 << PORF))    Serial.print(F("POWER-ON"));
  else                                  Serial.print(F("(none)"));
  Serial.print(F(" (0x"));
  if (resetFlags < 0x10) Serial.print('0');
  Serial.print(resetFlags, HEX);
  Serial.println(')');

  Serial.println(F("\nBooting " FIRMWARE_BUILD_INFO "..."));

  // Ustawienie timeoutu dla magistrali I2C, aby uniknąć zawieszenia programu.
  Wire.setWireTimeout(I2C_TIMEOUT_US, true);

  // ZMIANA: Odczyt konfiguracji sprzętowej z przełączników DIP
  dipReader.begin();
  // Poniższa linia odczytuje stan z fizycznych przełączników. Zakomentuj ją, jeśli chcesz symulować wartości.
  // uint16_t dipState = dipReader.readSwitches(); 
  // Poniższa linia symuluje stan przełączników. Każdy bit to jeden przełącznik.
  // ZMIANA: Bardzo czytelny sposób symulacji. Zmień '0' na '1', aby "włączyć" dany przełącznik.
  uint16_t dipState = (0 << DIP_SLEEP_MODE_ENABLED) |             // Bit 0: Włącza tryb oszczędzania energii
                      (0 << DIP_TRACKER_LDR_CALIBRATION) |        // Bit 1: Uruchamia kalibrację LDR przy starcie
                      (1 << DIP_TRACKER_SERVO_CALIBRATION) |    // Bit 2: Uruchamia kalibrację serw przy starcie
                      (1 << DIP_TRACKER_INITIAL_SEARCH) |       // Bit 3: Uruchamia wyszukiwanie słońca po starcie
                      (1 << DIP_TRACKER_USE_JOYSTICK) |         // Bit 4: Włącza sterowanie joystickiem
                      (1 << DIP_TRACKER_LDR_SENSORS_CONNECTED) | // Bit 5: Informuje, że fotorezystory są podłączone
                      (1 << DIP_TRACKER_ENABLE_SERVO_MOVEMENT) | // Bit 6: Globalna blokada ruchu serwomechanizmów trackera
                      (0 << DIP_TRACKER_ENABLE_DEBUG_PRINT);     // Bit 7: Włącza szczegółowe logi z SunTracker
  
  // Mapowanie bitów na flagi konfiguracyjne
  // ZMIANA: Używamy czytelnego enuma do sprawdzania bitów
  g_runtimeFlags.sleepModeEnabled             = (dipState & (1 << DIP_SLEEP_MODE_ENABLED));
  g_runtimeFlags.trackerPerformLdrCalibration = (dipState & (1 << DIP_TRACKER_LDR_CALIBRATION));
  g_runtimeFlags.trackerPerformServoCalibration = (dipState & (1 << DIP_TRACKER_SERVO_CALIBRATION));
  g_runtimeFlags.trackerPerformInitialSearch    = (dipState & (1 << DIP_TRACKER_INITIAL_SEARCH));
  g_runtimeFlags.trackerUseJoystick             = (dipState & (1 << DIP_TRACKER_USE_JOYSTICK));
  g_runtimeFlags.trackerLdrSensorsConnected     = (dipState & (1 << DIP_TRACKER_LDR_SENSORS_CONNECTED));
  g_runtimeFlags.trackerEnableServoMovement     = (dipState & (1 << DIP_TRACKER_ENABLE_SERVO_MOVEMENT));
  g_runtimeFlags.trackerEnableDebugPrint        = (dipState & (1 << DIP_TRACKER_ENABLE_DEBUG_PRINT));
  // Bity 8-15 są wolne do wykorzystania

  // Zastosowanie odczytanych flag w konfiguracji trackera
  const_cast<SunTrackerConfig&>(trackerConfig).performLdrCalibration = g_runtimeFlags.trackerPerformLdrCalibration;
  const_cast<SunTrackerConfig&>(trackerConfig).performServoCalibration = g_runtimeFlags.trackerPerformServoCalibration;
  const_cast<SunTrackerConfig&>(trackerConfig).performInitialSearch = g_runtimeFlags.trackerPerformInitialSearch;
  const_cast<SunTrackerConfig&>(trackerConfig).useJoystick = g_runtimeFlags.trackerUseJoystick;
  const_cast<SunTrackerConfig&>(trackerConfig).ldrSensorsConnected = g_runtimeFlags.trackerLdrSensorsConnected;
  const_cast<SunTrackerConfig&>(trackerConfig).enableServoMovement = g_runtimeFlags.trackerEnableServoMovement;
  const_cast<SunTrackerConfig&>(trackerConfig).enableDebugPrint = g_runtimeFlags.trackerEnableDebugPrint;

  // ZMIANA: Inicjalizacja LCD na początku, aby wyświetlać status uruchamiania
  lcd.init();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Boot: ");
  lcd.setCursor(0, 1);
  lcd.print(FIRMWARE_BUILD_INFO);
  // Pokazujemy przyczynę ostatniego resetu na LCD (gdy Serial niedostępny).
  lcd.setCursor(0, 2);
  lcd.print("Reset: ");
  if (!resetFlagsReliable) {
    lcd.print("(invalid)");
  } else if (resetFlags & (1 << WDRF))  lcd.print("WATCHDOG");
  else if (resetFlags & (1 << BORF))    lcd.print("BROWN-OUT");
  else if (resetFlags & (1 << EXTRF))   lcd.print("EXTERNAL");
  else if (resetFlags & (1 << PORF))    lcd.print("POWER-ON");
  else                                  lcd.print("(none)");
  // 4. linia: surowa wartość MCUSR (0xXX).
  char rawBuf[16];
  snprintf(rawBuf, sizeof(rawBuf), "MCUSR=0x%02X", resetFlags);
  lcd.setCursor(0, 3);
  lcd.print(rawBuf);
  delay(3000);
  lcd.clear();

  if (g_runtimeFlags.sleepModeEnabled) {
    Serial.println(F("Tryb oszczędzania energii WŁĄCZONY."));
    powerManager.begin(clock, lcd, led, sensor, dhtSensor);
  } else {
    Serial.println(F("Tryb oszczędzania energii WYŁĄCZONY. System będzie działał w trybie ciągłym."));
    pinMode(POWER_CONTROL_PIN, OUTPUT);
    powerManager.powerUpPeripherals();
  }
  
  // --- ZBIERANIE STATUSÓW INICJALIZACJI ---
  auto& rtcStatus = g_moduleStatuses[static_cast<int>(ModuleID::RTC)];
  if (clock.init()) {
    Serial.println(F("Zegar RTC OK."));
    rtcStatus.health = ModuleHealth::OK;
    strcpy(rtcStatus.statusText, "OK");
    clock.configureForAlarm();
    clock.clearAlarm(1);
    if (clock.lostPower()) {
      Serial.println(F("RTC stracił zasilanie! Ustawiam czas na czas kompilacji."));
      rtcStatus.health = ModuleHealth::WARNING;
      strcpy(rtcStatus.statusText, "SYNC...");
      clock.adjust(DateTime(F(__DATE__), F(__TIME__)));
      strcpy(rtcStatus.statusText, "OK (Synced)");
    }
  } else {
    Serial.println(F("Błąd inicjalizacji zegara RTC!"));
    rtcStatus.health = ModuleHealth::ERROR;
    strcpy(rtcStatus.statusText, "FAIL");
  }

  auto& bmeStatus = g_moduleStatuses[static_cast<int>(ModuleID::BME280)];
  if (sensor.init()) {
    Serial.println(F("Czujnik BME280 OK."));
    bmeStatus.health = ModuleHealth::OK;
    strcpy(bmeStatus.statusText, "OK");
  } else {
    Serial.println(F("Błąd inicjalizacji czujnika BME280!"));
    bmeStatus.health = ModuleHealth::ERROR;
    strcpy(bmeStatus.statusText, "FAIL");
  }

  auto& dhtStatus = g_moduleStatuses[static_cast<int>(ModuleID::DHT11)];
  dhtSensor.init();
  Serial.println(F("Czujnik DHT11 zainicjalizowany."));
  dhtStatus.health = ModuleHealth::OK;
  strcpy(dhtStatus.statusText, "OK");

  auto& panelStatus = g_moduleStatuses[static_cast<int>(ModuleID::CONTROL_PANEL)];
  controlPanel.begin();
  Serial.println(F("Panel sterowania zainicjalizowany."));
  panelStatus.health = ModuleHealth::OK;
  strcpy(panelStatus.statusText, "OK");

  auto& sdStatus = g_moduleStatuses[static_cast<int>(ModuleID::SD_CARD)];
  if (sdCard.init()) {
    sdStatus.health = ModuleHealth::OK;
    strcpy(sdStatus.statusText, "OK");
    if (sdCard.readConfiguration("config.txt", g_config)) {
      Serial.println(F("Konfiguracja wczytana pomyślnie."));
      strcat(sdStatus.statusText, "+CFG");
    } else {
      Serial.println(F("Nie udało się wczytać konfiguracji, używam wartości domyślnych."));
      strcat(sdStatus.statusText, "+DEF");
    }
  } else {
    sdStatus.health = ModuleHealth::ERROR;
    strcpy(sdStatus.statusText, "FAIL");
  }

  powerManager.setActiveModeDuration(g_config.activeModeMinutes);
  sensorUpdateTimer.setInterval(g_config.sensorUpdateIntervalMs);
  led.init(g_config.ledBrightness);
  
  // ZMIANA: Musimy zaktualizować konfigurację trackera po wczytaniu wartości z karty SD.
  // Inaczej używałby on wartości domyślnej, a nie tej z pliku config.txt.
  const_cast<SunTrackerConfig&>(trackerConfig).runningUpdateIntervalMs = g_config.trackerUpdateIntervalMs;

  auto& servoStatus = g_moduleStatuses[static_cast<int>(ModuleID::EXTRA_SERVOS)];
  if (SERVO_COUNT > 0) {
    for (int i = 0; i < SERVO_COUNT; i++) {
      servos[i].begin(servoConfigs[i].pin, servoConfigs[i].name, 0);
    }
    servoStatus.health = ModuleHealth::OK;
    strcpy(servoStatus.statusText, "OK");
  } else {
    servoStatus.health = ModuleHealth::DISABLED;
    strcpy(servoStatus.statusText, "OFF");
  }

  auto& gpsStatus = g_moduleStatuses[static_cast<int>(ModuleID::GPS)];
  gpsStatus.health = ModuleHealth::INITIALIZING;
  strcpy(gpsStatus.statusText, "INIT...");

  sunTracker.begin();
  Serial.println("SunTracker zainicjalizowany.");
  Serial.println(F("Wyświetlacz LED zainicjalizowany."));

  // --- ZMIANA: Uporządkowana sekwencja startowa na LCD ---
  // 1. Przekazujemy statusy do modułu LCD i przełączamy na ekran statusu.
  //    W tym momencie _statusScreenPage jest resetowane do 0.
  lcd.setModuleStatuses(g_moduleStatuses, static_cast<int>(ModuleID::MODULE_COUNT));
  lcd.nextScreen(); // Przełącza z MAIN na STATUS

  // 2. Rysujemy i wyświetlamy pierwszą stronę statusów (strona 0).
  lcd.update(g_sensorData); // Ręczne wywołanie, aby narysować ekran
  delay(3000);

  // 3. Ręcznie przełączamy na drugą stronę statusów i ją rysujemy.
  lcd.nextStatusPage();
  lcd.clear(); // Czyścimy ekran przed narysowaniem nowej strony
  lcd.update(g_sensorData);
  delay(3000);

  lcd.printWelcomeMessage();
  soundPlayer.playStartupSound();
  delay(3000);
  lcd.showMainScreen();

  // Backlight zostaje włączony jeszcze przez `backlight_boot_ms` po końcu setup,
  // potem updateBacklightTimer() zgasi go automatycznie. Następnie tylko touch
  // (lub wybudzenie ze snu) ponownie zapala podświetlenie.
  enableBacklightFor(g_config.backlightBootDurationMs);

  // ZMIANA: Włączamy Watchdog Timer na końcu setup z timeoutem 2 sekund.
  // Serial.println(F("Inicjalizacja zakończona. Włączam Watchdog Timer (2s)..."));
  // wdt_enable(WDTO_2S);
}

// --- Prywatna funkcja pomocnicza do obsługi logiki w trybie aktywnym ---
void handleActiveMode() {
  // --- Touch sensor: short press = backlight 30s (+ reset timera aktywności gdy sleep enabled),
  //     long press >= long_press_ms = force sleep (gdy sleep enabled). ---
  static unsigned long touchPressStartMs = 0;
  static bool longPressFired = false;

  touchSensor.update();

  if (g_skipTouchUntilRelease) {
    // Czekamy na puszczenie palca po wybudzeniu — nie traktujemy tego jako press.
    if (!touchSensor.isPressed()) {
      g_skipTouchUntilRelease = false;
      touchPressStartMs = 0;
      longPressFired = false;
    }
  } else if (touchSensor.isPressed()) {
    if (touchPressStartMs == 0) {
      touchPressStartMs = millis();
      longPressFired = false;
    } else if (!longPressFired &&
               (millis() - touchPressStartMs >= g_config.longPressThresholdMs)) {
      longPressFired = true;
      if (g_runtimeFlags.sleepModeEnabled) {
        powerManager.forceSleep();
      }
      // gdy sleepModeEnabled=false → no-op (sleep wyłączony)
    }
  } else {
    if (touchPressStartMs != 0 && !longPressFired) {
      // krótkie tknięcie
      enableBacklightFor(g_config.backlightTouchDurationMs);
      if (g_runtimeFlags.sleepModeEnabled) {
        powerManager.resetActiveTimer();
      }
    }
    touchPressStartMs = 0;
    longPressFired = false;
  }

  // Cykliczny odczyt czujników i aktualizacja danych
  if (sensorUpdateTimer.isReady()) {
    sensor.readData();
    dhtSensor.readData();

    DateTime now = clock.getTime();

    Clock::formatDate(now, g_sensorData.dateStr, sizeof(g_sensorData.dateStr));
    Clock::formatTime(now, g_sensorData.timeForLcd, sizeof(g_sensorData.timeForLcd), true);
    g_sensorData.hour = now.hour();
    g_sensorData.minute = now.minute();

    g_sensorData.temp_bme = sensor.getTemperature();
    g_sensorData.hum_bme = sensor.getHumidity();
    g_sensorData.pressure_bme = sensor.getPressure();
    g_sensorData.temp_rtc = clock.getTemperature();
    g_sensorData.temp_dht = dhtSensor.getTemperature();
    g_sensorData.hum_dht = dhtSensor.getHumidity();

    // ZMIANA: Pobieramy dane dla ekranu trackera
    g_sensorData.servo_h_pos = sunTracker.getHorizontalServoPosition();
    g_sensorData.servo_v_pos = sunTracker.getVerticalServoPosition();
    sunTracker.getLdrValues(g_sensorData.ldr_tl, g_sensorData.ldr_tr, g_sensorData.ldr_dl, g_sensorData.ldr_dr);

    // ZMIANA: Pobieramy dane z modułu GPS
    g_sensorData.gps_is_valid = gps.isDataValid();
    g_sensorData.gps_lat = gps.getLatitude();
    g_sensorData.gps_lon = gps.getLongitude();
    g_sensorData.gps_alt = gps.getAltitude();
    g_sensorData.gps_sats = gps.getSatellites();
    g_sensorData.gps_time_valid = gps.isDateTimeValid();
    g_sensorData.gps_year = gps.getYear();
    g_sensorData.gps_month = gps.getMonth();
    g_sensorData.gps_day = gps.getDay();
    g_sensorData.gps_hour = gps.getHour();
    g_sensorData.gps_minute = gps.getMinute();
    g_sensorData.gps_second = gps.getSecond();
    g_sensorData.gps_speed_kph = gps.getSpeedKph();
    g_sensorData.gps_speed_kts = gps.getSpeedKts();
    g_sensorData.gps_heading = gps.getHeading();

    // Aktualizacja statusu GPS na podstawie aktywności linii UART (devplan005)
    auto& gpsStatus = g_moduleStatuses[static_cast<int>(ModuleID::GPS)];
    GPSHealth gpsHealth = gps.getHealth(g_config.gpsNoDataTimeoutMs, g_config.gpsBadDataTimeoutMs);
    switch (gpsHealth) {
      case GPSHealth::FIXED:
        gpsStatus.health = ModuleHealth::OK;
        snprintf(gpsStatus.statusText, sizeof(gpsStatus.statusText), "FIXED (%d)", g_sensorData.gps_sats);
        break;
      case GPSHealth::SEARCHING:
        gpsStatus.health = ModuleHealth::WARNING;
        strcpy(gpsStatus.statusText, "SEARCHING");
        break;
      case GPSHealth::BAD_DATA:
        gpsStatus.health = ModuleHealth::ERROR;
        strcpy(gpsStatus.statusText, "BAD DATA");
        break;
      case GPSHealth::NO_MODULE:
        gpsStatus.health = ModuleHealth::ERROR;
        strcpy(gpsStatus.statusText, "NO MODULE");
        break;
    }


    lcd.update(g_sensorData);
    sdCard.logSensorData(g_sensorData, "datalog.txt");
  }

  // Aktualizacja wyświetlacza LED
  if (ledUpdateTimer.isReady()) {
    led.update(g_sensorData.hour, g_sensorData.minute);
  }
}

// --- Prywatna funkcja pomocnicza do raportowania stanu przez port szeregowy ---
void printStatusReport() {
  Serial.println(F("\nHEARTBEAT (Aktywny)\n"));

  int ldr_tl, ldr_tr, ldr_dl, ldr_dr;
  sunTracker.getLdrValues(ldr_tl, ldr_tr, ldr_dl, ldr_dr);

  char buffer[128];
  snprintf(buffer, sizeof(buffer),
           "Czas: %s | Temp(Z/N): %.1f/%.1fC | Wilg(Z/N): %.0f/%.0f%% | Cisn: %.1fhPa", g_sensorData.timeForLcd,
           (double)g_sensorData.temp_bme, (double)g_sensorData.temp_dht,
           (double)g_sensorData.hum_bme, (double)g_sensorData.hum_dht, (double)g_sensorData.pressure_bme);
  Serial.println(buffer);
  snprintf(buffer, sizeof(buffer),
           "Serva(H/V): %d/%d | LDR(TL,TR,DL,DR): %d,%d,%d,%d",
           sunTracker.getHorizontalServoPosition(), sunTracker.getVerticalServoPosition(),
           ldr_tl, ldr_tr, ldr_dl, ldr_dr);
  Serial.println(buffer);
  
  Serial.println(); // Pusta linia dla czytelności
}


void loop() {
  commandHandler.update(); // Sprawdzaj, czy przyszła komenda synchronizacji

  controlPanel.update(); // Odczytuj stan joysticków, enkodera i przycisków

  // Obsługa przełączania ekranów enkoderem — każda interakcja zapala backlight na backlight_encoder_ms.
  int encoderChange = controlPanel.getEncoderChange();
  if (encoderChange > 0) {
    lcd.nextScreen();
    enableBacklightFor(g_config.backlightEncoderDurationMs);
  } else if (encoderChange < 0) {
    lcd.previousScreen();
    enableBacklightFor(g_config.backlightEncoderDurationMs);
  }

  // Klik enkodera → powrót na ekran główny + zapal backlight.
  if (controlPanel.wasEncoderClicked()) {
    lcd.showMainScreen();
    enableBacklightFor(g_config.backlightEncoderDurationMs);
  }
  // wdt_reset();

  // ZMIANA: Aktualizujemy stan modułu GPS w każdej pętli
  // To kluczowe dla TinyGPS++, aby mogła ona ciągle przetwarzać dane z portu szeregowego.
  gps.update();

  // Pomijamy sunTracker.update() gdy tracker jest "odłączony" — czyli LDR-y
  // nie są podpięte LUB ruch serw jest wyłączony (DIP bity 5 i 6).
  // Bez LDR nie ma czego mierzyć, bez serw nie ma czego ruszać.
  if (g_runtimeFlags.trackerLdrSensorsConnected && g_runtimeFlags.trackerEnableServoMovement) {
    sunTracker.update();
  }
  // wdt_reset();

  // Mruganie wbudowaną diodą LED jako "heartbeat" systemu
  if (sdCard.isOK()) {
    // Normalny "heartbeat" systemu
    if (builtinLedTimer.isReady()) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  } else {
    // Szybkie mruganie jako sygnalizacja błędu (np. pełna karta SD)
    if (errorLedTimer.isReady()) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }

  // PowerManager.update() przetwarzane TYLKO gdy sleep mode włączony (DIP bit 0 = 1).
  // Bez tego guarda timer i tak doliczał do końca i wchodził w PREPARE_SLEEP, co wywoływało
  // crash przez I2C-after-deenergize w prepareToSleep() — patrz log 1.3.4.
  if (g_runtimeFlags.sleepModeEnabled) {
    powerManager.update();
  }
  // wdt_reset();

  // Po wybudzeniu (RTC alarm lub touch) — backlight ON na okno boot (60s domyślnie).
  // Dodatkowo, jeśli user trzyma palec po wybudzeniu, blokujemy detekcję long-press
  // do następnego release przez touchSensor (consume jednorazowego wasPressed eventu nie wystarczy,
  // bo używamy isPressed() — ale flaga touchPressStartMs zostanie ustawiona dopiero w handleActiveMode).
  if (powerManager.consumeWakeEvent()) {
    enableBacklightFor(g_config.backlightBootDurationMs);
    g_skipTouchUntilRelease = true;
  }

  // Wygaszanie podświetlenia po upływie zaplanowanego okna.
  updateBacklightTimer();
  
  // Aktualizacja stanu serwomechanizmów (musi być wywoływana w każdej pętli)
  for (int i = 0; i < SERVO_COUNT; i++) {
    if (servos[i].update()) {
      // Serwo jest w ruchu, można coś z tym zrobić, jeśli potrzeba
    }
  }
  // wdt_reset();

  if (!g_runtimeFlags.sleepModeEnabled || powerManager.isAwake()) {
    // Wywołujemy nową, wydzieloną funkcję
    handleActiveMode();
  // wdt_reset();
    
    if (heartbeatTimer.isReady()) { 
      // printStatusReport();
      // // ZMIANA: Dodajemy cykliczne wyświetlanie stanu panelu sterowania
      // controlPanel.printDebugInfo();
      //wdt_reset();
    }
  }

  // ZMIANA: "Głaskanie" Watchdoga. Resetujemy jego licznik w każdej pętli,
  // sygnalizując, że program działa poprawnie.
  // wdt_reset();
}
