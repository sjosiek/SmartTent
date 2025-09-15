// Plik: ProjectConfig.h
// Centralne miejsce do definicji pinów i podstawowych stałych konfiguracyjnych.

#ifndef HARDWARE_PINS_H
#define HARDWARE_PINS_H

#include <Arduino.h>
#include <DHT.h> // Potrzebne dla definicji DHT_TYPE

// --- Ustawienia globalne ---
constexpr bool SLEEP_MODE_ENABLED = false;

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
constexpr bool TRACKER_PERFORM_LDR_CALIBRATION = false;
constexpr bool TRACKER_PERFORM_SERVO_CALIBRATION = true;
constexpr bool TRACKER_PERFORM_INITIAL_SEARCH = true;
constexpr bool TRACKER_USE_JOYSTICK = true;
constexpr bool TRACKER_LDR_SENSORS_CONNECTED = true;
constexpr bool TRACKER_ENABLE_SERVO_MOVEMENT = true;

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

#endif // HARDWARE_PINS_H