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
#include "SensorData.h"      // Dołączamy strukturę danych

const bool SLEEP_MODE_ENABLED = true;


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

#define DHT_TYPE DHT11          // Typ czujnika DHT11

#define LCD_ADDRESS 0x27        // Adres wyświetlacza LCD
#define LCD_COLS 20             // Liczba kolumn wyświetlacza
#define LCD_ROWS 4              // Liczba wierszy wyświetlacza

// Definicja timeoutu dla magistrali I2C w mikrosekundach.
// Zapobiega to zawieszeniu się programu, gdy urządzenie I2C nagle straci zasilanie.
const uint32_t I2C_TIMEOUT_US = 25000; // 25000 mikrosekund = 25 milisekund

// Tablica pinów danych, które muszą być de-energetyzowane przed uśpieniem
const uint8_t DATA_PINS_TO_DEENERGIZE[] = {
    20, 21,             // I2C: SDA, SCL
    DHT_PIN,            // DHT11
    LED_CLK_PIN,        // LED Display
    LED_DIO_PIN,        // LED Display
    SPI_MISO_PIN, SPI_MOSI_PIN, SPI_SCK_PIN, SD_CS_PIN // SPI dla karty SD
};
const uint8_t DATA_PINS_COUNT = sizeof(DATA_PINS_TO_DEENERGIZE) / sizeof(DATA_PINS_TO_DEENERGIZE[0]);


//Inicjalizacja modułów

Clock clock;                   // RTC
DhtSensor dhtSensor(DHT_PIN, DHT_TYPE);          // DHT11
WeatherSensor sensor;          //BME 280

LcdDisplay lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);  // LCD
LedDisplay led(LED_CLK_PIN, LED_DIO_PIN);         // LED

// Czujnik dotykowy jest aktywny stanem wysokim (nie ma zworek do zmiany logiki).
DebouncedButton touchSensor(TOUCH_SENSOR_PIN, ActiveState::ACTIVE_HIGH);

PowerManager powerManager(POWER_CONTROL_PIN, RTC_ALARM_PIN, TOUCH_SENSOR_PIN, DATA_PINS_TO_DEENERGIZE, DATA_PINS_COUNT);
CommandHandler commandHandler(clock);
SDCard sdCard(SD_CS_PIN);


Timer sensorUpdateTimer(1000); 
Timer ledUpdateTimer(500);      
Timer heartbeatTimer(5000);
Timer builtinLedTimer(1000); // Timer do mrugania wbudowaną diodą LED

SensorData g_sensorData; // Zastępujemy wiele zmiennych globalnych jedną strukturą

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
    
  }

  // Inicjalizacja karty SD (zawsze, niezależnie od trybu uśpienia)
  sdCard.init();
}

void loop() {
  commandHandler.update(); // Sprawdzaj, czy przyszła komenda synchronizacji

  // Mruganie wbudowaną diodą LED jako "heartbeat" systemu
  if (builtinLedTimer.isReady()) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  if (SLEEP_MODE_ENABLED) {
    powerManager.update();
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
      Serial.print(g_sensorData.pressure_bme);
    }
  }
}