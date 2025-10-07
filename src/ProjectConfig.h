// Plik: ProjectConfig.h
// Centralne miejsce do definicji pinów i podstawowych stałych konfiguracyjnych.

#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

#include <Arduino.h>
#include <DHT.h> // Potrzebne dla definicji DHT_TYPE

struct Configuration {
  uint32_t activeModeMinutes = 5;       // Domyślnie 5 minut
  uint32_t sensorUpdateIntervalMs = 1000; // Domyślnie 1 sekunda (dla LCD, SD)
  uint32_t trackerUpdateIntervalMs = 300000; // Domyślnie 5 minut (dla SunTracker)
  uint8_t ledBrightness = 7;            // Domyślnie 7
};

// NOWA STRUKTURA: Przechowuje flagi konfiguracyjne odczytywane przy starcie
struct RuntimeFlags {
  bool sleepModeEnabled;
  bool trackerPerformLdrCalibration;
  bool trackerPerformServoCalibration;
  bool trackerPerformInitialSearch;
  bool trackerUseJoystick;
  bool trackerLdrSensorsConnected;
  bool trackerEnableServoMovement;
  bool trackerEnableDebugPrint;
  // Można tu dodać kolejne 8 flag dla drugiego przełącznika
};

// NOWOŚĆ: Enum do mapowania bitów na funkcje dla większej czytelności
enum DipSwitchBits {
  DIP_SLEEP_MODE_ENABLED = 0,
  DIP_TRACKER_LDR_CALIBRATION = 1,
  DIP_TRACKER_SERVO_CALIBRATION = 2,
  DIP_TRACKER_INITIAL_SEARCH = 3,
  DIP_TRACKER_USE_JOYSTICK = 4,
  DIP_TRACKER_LDR_SENSORS_CONNECTED = 5,
  DIP_TRACKER_ENABLE_SERVO_MOVEMENT = 6,
  DIP_TRACKER_ENABLE_DEBUG_PRINT = 7
  // Bity 8-15 wolne
};

// --- Ustawienia globalne ---
// USUNIĘTO: constexpr bool SLEEP_MODE_ENABLED = false; - teraz będzie w RuntimeFlags

// --- Magistrala I2C ---
constexpr uint8_t LCD_ADDRESS = 0x27;
constexpr uint8_t LCD_COLS = 20;
constexpr uint8_t LCD_ROWS = 4;
// Adres BME280 jest domyślny w klasie (0x76), więc nie ma potrzeby go tu definiować.

// --- Magistrala SPI ---
// Piny MISO (50), MOSI (51) i SCK (52) są standardowymi pinami sprzętowymi SPI
// dla Arduino Mega i są automatycznie definiowane przez biblioteki.
constexpr uint8_t SD_CS_PIN = 53;

// --- Przerwania ---
constexpr uint8_t RTC_ALARM_PIN = 2;    // Przerwanie 0
constexpr uint8_t TOUCH_SENSOR_PIN = 3; // Przerwanie 1

// --- Zasilanie ---
constexpr uint8_t POWER_CONTROL_PIN = 4;

// --- Czujniki ---
constexpr uint8_t DHT_PIN = 6;
constexpr uint8_t DHT_TYPE = DHT11;

// --- Wyświetlacze ---
constexpr uint8_t LED_CLK_PIN = 22;
constexpr uint8_t LED_DIO_PIN = 23;

// --- Sun Tracker ---
constexpr uint8_t HORIZONTAL_SERVO_PIN = 9;
constexpr uint8_t VERTICAL_SERVO_PIN = 10;
constexpr uint8_t LDR_TOP_LEFT_PIN = A1;
constexpr uint8_t LDR_TOP_RIGHT_PIN = A2;
constexpr uint8_t LDR_DOWN_LEFT_PIN = A0;
constexpr uint8_t LDR_DOWN_RIGHT_PIN = A3;

// --- Sun Tracker - Konfiguracja startowa ---
// USUNIĘTO: Wszystkie flagi constexpr dla trackera, zostaną zastąpione przez RuntimeFlags

// --- Piny dla czytnika DIP Switch (74HC165) ---
constexpr uint8_t DIP_LATCH_PIN = 30;
constexpr uint8_t DIP_CLOCK_PIN = 31;
constexpr uint8_t DIP_DATA_PIN  = 32;

// --- Control Panel ---
constexpr uint8_t JOY1_X_PIN = A8;
constexpr uint8_t JOY1_Y_PIN = A9;
constexpr uint8_t JOY1_SW_PIN = 24;
constexpr uint8_t JOY2_X_PIN = A10;
constexpr uint8_t JOY2_Y_PIN = A11;
constexpr uint8_t JOY2_SW_PIN = 25;
constexpr uint8_t ENC_DT_PIN = 26;
constexpr uint8_t ENC_CLK_PIN = 27;
constexpr uint8_t ENC_SW_PIN = 28;
constexpr uint8_t BUZZER_PIN = 29;

// --- Dodatkowe potencjometry ---
constexpr uint8_t POT1_PIN = A12;
constexpr uint8_t POT2_PIN = A13;
constexpr uint8_t POT3_PIN = A14;
constexpr uint8_t POT4_PIN = A15;

#endif // HARDWARE_PINS_H