// Plik główny: main.cpp
// Wersja z poprawioną logiką w funkcji setup()

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
    TRACKER_PERFORM_LDR_CALIBRATION,
    TRACKER_PERFORM_SERVO_CALIBRATION,
    TRACKER_PERFORM_INITIAL_SEARCH,
    TRACKER_USE_JOYSTICK,
    false, // usePotentiometers (nieużywane, ale musi być w inicjalizatorze)
    TRACKER_LDR_SENSORS_CONNECTED,
    TRACKER_ENABLE_SERVO_MOVEMENT,
    TRACKER_ENABLE_DEBUG_PRINT,
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
    A4, A5,             // I2C: SDA, SCL
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
  BUZZER_PIN
};

ControlPanel controlPanel(controlPanelPins);


//Inicjalizacja modułów
SunTracker sunTracker(trackerPins, trackerConfig, controlPanel);  //SUnTracker

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
 
CommandHandler commandHandler(clock, lcd, sdCard, g_config, servos, SERVO_COUNT); // Przekazujemy tablicę do CommandHandler
 
 
Timer sensorUpdateTimer(1000); // Domyślny interwał, zostanie nadpisany przez konfigurację z SD
Timer ledUpdateTimer(500);
Timer heartbeatTimer(1000);  // ZMIANA: Heartbeat co 1 sekundę
Timer builtinLedTimer(1000); // Timer do mrugania wbudowaną diodą LED
Timer errorLedTimer(200);    // Szybszy timer do sygnalizacji błędu



void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Inicjalizacja wbudowanej diody LED
  Serial.begin(9600);
  Serial.println(F("\nBooting SmartTent System..."));

  // Ustawienie timeoutu dla magistrali I2C, aby uniknąć zawieszenia programu.
  Wire.setWireTimeout(I2C_TIMEOUT_US, true);

  // ZMIANA: Inicjalizacja LCD na początku, aby wyświetlać status uruchamiania
  lcd.init();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Boot: SmartTent...");

  if (SLEEP_MODE_ENABLED) {
    Serial.println(F("Tryb oszczędzania energii WŁĄCZONY."));
    powerManager.begin(clock, lcd, led, sensor, dhtSensor);
  } else {
    Serial.println(F("Tryb oszczędzania energii WYŁĄCZONY. System będzie działał w trybie ciągłym."));
    pinMode(POWER_CONTROL_PIN, OUTPUT);
    powerManager.powerUpPeripherals();
  }
  
  // ZMIANA: Wyświetlanie statusu inicjalizacji na LCD
  if (clock.init()) {
    Serial.println(F("Zegar RTC OK."));
    lcd.printStatus("Zegar RTC", "OK", 1);
    clock.configureForAlarm();
    clock.clearAlarm(1);
    if (clock.lostPower()) {
      Serial.println(F("RTC stracił zasilanie! Ustawiam czas na czas kompilacji."));
      // ZMIANA: Aktualizujemy status na LCD, informując o synchronizacji
      lcd.printStatus("Zegar RTC", "LOST POWER -> SYNC", 1);
      clock.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  } else {
    Serial.println(F("Błąd inicjalizacji zegara RTC!"));
    lcd.printStatus("Zegar RTC", "FAIL", 1);
  }

  if (sensor.init()) {
    Serial.println(F("Czujnik BME280 OK."));
    lcd.printStatus("BME280", "OK", 2);
  } else {
    Serial.println(F("Błąd inicjalizacji czujnika BME280!"));
    lcd.printStatus("BME280", "FAIL", 2);
  }

  dhtSensor.init();
  // Dodajemy informację o statusie DHT11 na LCD
  lcd.printStatus("DHT11", "OK", 3); // ZMIANA: Przeniesiono do wiersza 3
  Serial.println(F("Czujnik DHT11 zainicjalizowany."));

  // Inicjalizacja panelu sterowania (zawsze, niezależnie od trybu)
  controlPanel.begin();
  Serial.println(F("Panel sterowania zainicjalizowany."));

  // ZMIANA: Dodajemy informację o gotowości CommandHandler
  lcd.printStatus("Cmd Handler", "OK", 3);

  // Inicjalizacja karty SD (zawsze, niezależnie od trybu uśpienia)
  if (sdCard.init()) {
    lcd.printStatus("Karta SD", "OK", 0); // ZMIANA: Przeniesiono do wiersza 0
  } else {
    lcd.printStatus("Karta SD", "FAIL", 0); // ZMIANA: Przeniesiono do wiersza 0
  }

  // Odczyt pliku konfiguracyjnego
  if (sdCard.readConfiguration("config.txt", g_config)) {
    Serial.println(F("Konfiguracja wczytana pomyślnie."));
  } else {
    Serial.println(F("Nie udało się wczytać konfiguracji, używam wartości domyślnych."));
  }

  // Zastosowanie wczytanej konfiguracji
  powerManager.setActiveModeDuration(g_config.activeModeMinutes);
  sensorUpdateTimer.setInterval(g_config.sensorUpdateIntervalMs);
  led.init(g_config.ledBrightness);
  
  // ZMIANA: Musimy zaktualizować konfigurację trackera po wczytaniu wartości z karty SD.
  // Inaczej używałby on wartości domyślnej, a nie tej z pliku config.txt.
  const_cast<SunTrackerConfig&>(trackerConfig).runningUpdateIntervalMs = g_config.trackerUpdateIntervalMs;

  // Inicjalizacja serwomechanizmów
  for (int i = 0; i < SERVO_COUNT; i++) {
    // Inicjalizujemy każde serwo na podstawie centralnej konfiguracji
    // Można tu dodać logikę ustawiania różnych pozycji startowych, np. z pliku konfiguracyjnego
    servos[i].begin(servoConfigs[i].pin, servoConfigs[i].name, 0);
  }

  // Inicjalizacja pozostałych modułów, które nie zwracają statusu
  sunTracker.begin();
  Serial.println("SunTracker zainicjalizowany.");
  led.init(g_config.ledBrightness);
  Serial.println(F("Wyświetlacz LED zainicjalizowany."));

  delay(2000); // Czas na odczytanie statusu
  lcd.printWelcomeMessage();
}

// --- Prywatna funkcja pomocnicza do obsługi logiki w trybie aktywnym ---
void handleActiveMode() {
  // Obsługa przycisku dotykowego do resetowania timera uśpienia
  if (SLEEP_MODE_ENABLED) {
    touchSensor.update();
    if (touchSensor.wasPressed()) {
      powerManager.resetActiveTimer();
    }
  }

  // Cykliczny odczyt czujników i aktualizacja danych
  if (sensorUpdateTimer.isReady()) {
    sensor.readData();
    dhtSensor.readData();

    DateTime now = clock.getTime();
    g_sensorData.dateStr = Clock::formatDate(now);
    g_sensorData.timeForLcd = Clock::formatTime(now, true);
    g_sensorData.hour = now.hour();
    g_sensorData.minute = now.minute();

    g_sensorData.temp_bme = sensor.getTemperature();
    g_sensorData.hum_bme = sensor.getHumidity();
    g_sensorData.pressure_bme = sensor.getPressure();
    g_sensorData.temp_rtc = clock.getTemperature();
    g_sensorData.temp_dht = dhtSensor.getTemperature();
    g_sensorData.hum_dht = dhtSensor.getHumidity();

    lcd.update(g_sensorData);
    sdCard.logSensorData(g_sensorData, "datalog.txt");
  }

  // Aktualizacja wyświetlacza LED
  if (ledUpdateTimer.isReady()) {
    led.update(g_sensorData.hour, g_sensorData.minute);
  }
}

void loop() {
  commandHandler.update(); // Sprawdzaj, czy przyszła komenda synchronizacji

  controlPanel.update(); // Odczytuj stan joysticków, enkodera i przycisków
  sunTracker.update();

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
    // Wywołujemy nową, wydzieloną funkcję
    handleActiveMode();
    
    if (heartbeatTimer.isReady()) { 
      Serial.println(F("\nHEARTBEAT (Aktywny)\n"));

      int ldr_tl, ldr_tr, ldr_dl, ldr_dr;
      sunTracker.getLdrValues(ldr_tl, ldr_tr, ldr_dl, ldr_dr);

      char buffer[128];
      snprintf(buffer, sizeof(buffer),
               "Czas: %s | Temp(Z/N): %.1f/%.1fC | Wilg(Z/N): %.0f/%.0f%% | Cisn: %.1fhPa",
               g_sensorData.timeForLcd.c_str(), (double)g_sensorData.temp_bme, (double)g_sensorData.temp_dht,
               (double)g_sensorData.hum_bme, (double)g_sensorData.hum_dht, (double)g_sensorData.pressure_bme);
      Serial.println(buffer);
      snprintf(buffer, sizeof(buffer),
               "Serva(H/V): %d/%d | LDR(TL,TR,DL,DR): %d,%d,%d,%d",
               sunTracker.getHorizontalServoPosition(), sunTracker.getVerticalServoPosition(),
               ldr_tl, ldr_tr, ldr_dl, ldr_dr);
      Serial.println(buffer);
      
      Serial.println(); // Pusta linia dla czytelności
    }
  }
}
