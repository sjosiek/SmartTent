// Plik główny: main.cpp
// Wersja z poprawioną logiką w funkcji setup()

#include <SPI.h>
#include <SD.h>
#include <Arduino.h>
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
#include "Configuration.h"   // Dołączamy strukturę konfiguracyjną
#include "SmoothServo.h"     // Dołączamy klasę serwomechanizmu
#include "SensorData.h"      // Dołączamy strukturę danych
#include "SunTracker.h"      // SUnTracker

const bool SLEEP_MODE_ENABLED = false;

constexpr int HORIZONTAL_SERVO_PIN = 9; //poziome
constexpr int VERTICAL_SERVO_PIN = 10;  //pionowe
constexpr int LDR_TOP_LEFT_PIN = A0;
constexpr int LDR_TOP_RIGHT_PIN = A1;
constexpr int LDR_DOWN_LEFT_PIN = A2;
constexpr int LDR_DOWN_RIGHT_PIN = A3;
constexpr int JOYSTICK_X_PIN = A4; // Oś X joysticka
constexpr int JOYSTICK_Y_PIN = A5; // Oś Y joysticka
constexpr int JOYSTICK_SW_PIN = 35; // Przycisk joysticka

// --- Konfiguracja działania trackera---
const SunTrackerPins trackerPins = {
    .horizontalServoPin = HORIZONTAL_SERVO_PIN,
    .verticalServoPin = VERTICAL_SERVO_PIN,
    .ldrTopLeftPin = LDR_TOP_LEFT_PIN,
    .ldrTopRightPin = LDR_TOP_RIGHT_PIN,
    .ldrDownLeftPin = LDR_DOWN_LEFT_PIN,
    .ldrDownRightPin = LDR_DOWN_RIGHT_PIN,
    .joystickXPin = JOYSTICK_X_PIN,
    .joystickYPin = JOYSTICK_Y_PIN,
    .joystickSwPin = JOYSTICK_SW_PIN
};

const SunTrackerConfig trackerConfig = {
    .servoVMinAngle = 10,
    .servoVMaxAngle = 85,
    .servoHMinAngle = 5,
    .servoHMaxAngle = 175,
    .performLdrCalibration = true,
    .performServoCalibration = false,
    .performInitialSearch = false,
    .useJoystick = true,
    .usePotentiometers = false,
    .ldrSensorsConnected = true,
    .enableServoMovement = false,
    .defaultServoSpeed = 80,
    .defaultTolerance = 20,
    .runningUpdateIntervalMs = 1000 // 5 minut
};




#define RTC_ALARM_PIN 2         // Pin dla alarmu z RTC (Przerwanie 0) - SQW
#define TOUCH_SENSOR_PIN 3      // Pin dla czujnika dotykowego (Przerwanie 1)
#define POWER_CONTROL_PIN 4     // Pin do sterowania zasilaniem peryferiów
#define DHT_PIN 6               // Nowy pin dla czujnika DHT11
#define LED_CLK_PIN 8           // CLK pin dla wyświetlacza LED
#define LED_DIO_PIN 9           // DIO pin dla wyświetlacza LED
#define SPI_MISO_PIN 50         // Sprzętowy pin MISO dla SPI
#define SPI_MOSI_PIN 51         // Sprzętowy pin MOSI dla SPI
#define SPI_SCK_PIN 52          // Sprzętowy pin SCK dla SPI
#define SD_CS_PIN 53            // Pin Chip Select dla karty SD
// #define SERVO1_PIN A0           // Pin dla pierwszego serwa
// #define SERVO2_PIN A1           // Pin dla drugiego serwa

#define DHT_TYPE DHT11          // Typ czujnika DHT11

#define LCD_ADDRESS 0x27        // Adres wyświetlacza LCD
#define LCD_COLS 20             // Liczba kolumn wyświetlacza
#define LCD_ROWS 4              // Liczba wierszy wyświetlacza

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
    20, 21,             // I2C: SDA, SCL
    DHT_PIN,            // DHT11
    LED_CLK_PIN,        // LED Display
    LED_DIO_PIN,        // LED Display
    SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SCK_PIN, SD_CS_PIN, // SPI dla karty SD
    // SERVO1_PIN, SERVO2_PIN // Piny serwomechanizmów
    // UWAGA: Jeśli dodasz serwa, pamiętaj o dodaniu ich pinów tutaj!
};
const uint8_t DATA_PINS_COUNT = sizeof(DATA_PINS_TO_DEENERGIZE) / sizeof(DATA_PINS_TO_DEENERGIZE[0]);


//Inicjalizacja modułów

SunTracker sunTracker(trackerPins, trackerConfig);  //SUnTracker

Clock clock;                   // RTC
DhtSensor dhtSensor(DHT_PIN, DHT_TYPE);          // DHT11
WeatherSensor sensor;          //BME 280

LcdDisplay lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);  // LCD
LedDisplay led(LED_CLK_PIN, LED_DIO_PIN);         // LED

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
 
CommandHandler commandHandler(clock, sdCard, g_config, servos, SERVO_COUNT); // Przekazujemy tablicę do CommandHandler
 
 
Timer sensorUpdateTimer(1000); // Domyślny interwał, zostanie nadpisany przez konfigurację
Timer ledUpdateTimer(500);      
Timer heartbeatTimer(5000);
Timer builtinLedTimer(1000); // Timer do mrugania wbudowaną diodą LED
Timer errorLedTimer(200);    // Szybszy timer do sygnalizacji błędu


void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Inicjalizacja wbudowanej diody LED
  Serial.begin(9600);
  Serial.println("\nBooting SmartTent System...");

  // Ustawienie timeoutu dla magistrali I2C, aby uniknąć zawieszenia programu.
  Wire.setWireTimeout(I2C_TIMEOUT_US, true);

  if (!clock.init()) {
    Serial.println("Błąd inicjalizacji zegara RTC!");
    //while (1); // Zatrzymanie programu, krytyczny błąd. Odkomentuj w wersji finalnej.
  } else {
    Serial.println("Zegar RTC OK.");
    clock.configureForAlarm(); // KONIECZNIE: Konfigurujemy pin SQW do pracy jako przerwanie.
    // Na wszelki wypadek czyścimy flagę alarmu, gdyby system został zresetowany w trakcie jego trwania.
    clock.clearAlarm(1);
    // Sprawdzamy, czy zegar nie stracił zasilania i nie zresetował się do domyślnej daty
    if (clock.lostPower()) {
      Serial.println("RTC stracił zasilanie! Ustawiam czas na czas kompilacji.");
      // Poniższa linia ustawi czas na datę i godzinę kompilacji tego szkicu
      clock.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }
    
  if (SLEEP_MODE_ENABLED) {
    Serial.println("Tryb oszczędzania energii WŁĄCZONY.");
    powerManager.begin(clock, lcd, led, sensor, dhtSensor);
  } else {
    Serial.println("Tryb oszczędzania energii WYŁĄCZONY. System będzie działał w trybie ciągłym.");
    pinMode(POWER_CONTROL_PIN, OUTPUT);

    // ZMIANA: Używamy teraz publicznej metody z PowerManagera do włączenia zasilania
    powerManager.powerUpPeripherals();

    // Inicjalizujemy resztę modułów
    if (!sensor.init()) {
      Serial.println("Błąd inicjalizacji czujnika BME280!");
    } else {
      Serial.println("Czujnik BME280 OK.");
    }

    dhtSensor.init();
    Serial.println("Czujnik DHT11 zainicjalizowany.");

   
    lcd.init(); 
    Serial.println("Wyświetlacz LCD zainicjalizowany.");

    led.init(10);
    Serial.println("Wyświetlacz LED zainicjalizowany.");
    
    lcd.printWelcomeMessage();

    Serial.println("SunTracker zainicjalizowany.");

    sunTracker.begin();
    
  }

  // Inicjalizacja karty SD (zawsze, niezależnie od trybu uśpienia)
  sdCard.init();

  // Odczyt pliku konfiguracyjnego
  if (sdCard.readConfiguration("config.txt", g_config)) {
    Serial.println("Konfiguracja wczytana pomyślnie.");
  } else {
    Serial.println("Nie udało się wczytać konfiguracji, używam wartości domyślnych.");
  }

  // Zastosowanie wczytanej konfiguracji
  powerManager.setActiveModeDuration(g_config.activeModeMinutes);
  sensorUpdateTimer.setInterval(g_config.sensorUpdateIntervalMs);
  led.init(g_config.ledBrightness);

  // Inicjalizacja serwomechanizmów
  for (int i = 0; i < SERVO_COUNT; i++) {
    // Inicjalizujemy każde serwo na podstawie centralnej konfiguracji
    // Można tu dodać logikę ustawiania różnych pozycji startowych, np. z pliku konfiguracyjnego
    servos[i].begin(servoConfigs[i].pin, servoConfigs[i].name, 0);
  }
}

void loop() {
  commandHandler.update(); // Sprawdzaj, czy przyszła komenda synchronizacji

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

  if (SLEEP_MODE_ENABLED) {
    powerManager.update();
  }
  
  // Aktualizacja stanu serwomechanizmów (musi być wywoływana w każdej pętli)
  for (int i = 0; i < SERVO_COUNT; i++) {
    if (servos[i].update()) {
      // Serwo jest w ruchu, można coś z tym zrobić, jeśli potrzeba
    }
  }

  if (!SLEEP_MODE_ENABLED || powerManager.isAwake()) {
    
    // Obsługa przycisku za pomocą nowej, czystej klasy
    if (SLEEP_MODE_ENABLED) {
      touchSensor.update(); // Zawsze aktualizujemy stan przycisku
      
      if (touchSensor.wasPressed()) {
        powerManager.resetActiveTimer();
      }
    }

    
    if (sensorUpdateTimer.isReady()) { 
      sensor.readData(); 
      g_sensorData.temp_bme = sensor.getTemperature(); 
      g_sensorData.hum_bme = sensor.getHumidity();
      g_sensorData.pressure_bme = sensor.getPressure();
      g_sensorData.temp_rtc = clock.getTemperature();
      
      dhtSensor.readData();
      g_sensorData.temp_dht = dhtSensor.getTemperature();
      g_sensorData.hum_dht = dhtSensor.getHumidity();

      DateTime now = clock.getTime(); // Pobierz czas tylko raz
      g_sensorData.dateStr = Clock::formatDate(now);
      g_sensorData.timeForLcd = Clock::formatTime(now, true);
      g_sensorData.timeForLed = Clock::formatTime(now, false);
      
      // Zapisujemy pozycje pierwszych dwóch serw do wyświetlenia na LCD
      // UWAGA: Ta część nadal jest "na sztywno" dla 2 serw z powodu ograniczeń wyświetlacza.
      // Można to rozbudować o system przełączania ekranów.
      
      // g_sensorData.servo1_pos = (SERVO_COUNT > 0) ? servos[0].getCurrentPosition() : 0;
      // g_sensorData.servo2_pos = (SERVO_COUNT > 1) ? servos[1].getCurrentPosition() : 0;
      

      lcd.update(g_sensorData);
      
      // Zapisujemy dane na karcie SD przy każdym nowym odczycie
      sdCard.logSensorData(g_sensorData, "datalog.txt");
    }

    if (ledUpdateTimer.isReady()) { 
      if (g_sensorData.timeForLed.length() > 0) led.update(g_sensorData.timeForLed);
    }
    
    if (heartbeatTimer.isReady()) { 
      Serial.println("\nHEARTBEAT (Aktywny)\n");
      
      Serial.print("\nData:");
      Serial.print(g_sensorData.dateStr);
      Serial.print("\nCzas:");
      Serial.print(g_sensorData.timeForLcd);
      Serial.print("\nCzas LED:");
      Serial.print(g_sensorData.timeForLed);
      Serial.print("\nZewn: ");
      Serial.print(g_sensorData.temp_bme);
      Serial.print("C, Wilg(Z): ");
      Serial.print(g_sensorData.hum_bme);
      Serial.print("C, Wewn: ");
      Serial.print(g_sensorData.temp_rtc);
      Serial.print("C, Namiot: ");
      Serial.print(g_sensorData.temp_dht);
      Serial.print("C, Wilg(N): ");
      Serial.print(g_sensorData.hum_dht);
      Serial.print("%, Cisnienir(hPa): ");
      Serial.println(g_sensorData.pressure_bme, 2);
      
      sunTracker.update();
      
      Serial.println();
    }
  }
}
