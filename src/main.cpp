// Plik główny: main.cpp
// Wersja z poprawioną logiką w funkcji setup()

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

const bool SLEEP_MODE_ENABLED = true;


#define RTC_ALARM_PIN 2         // Pin dla alarmu z RTC (Przerwanie 0) - SQW
#define WAKEUP_INTERRUPT_PIN 3      // Pin dla czujnika dotykowego (Przerwanie 1)
#define POWER_CONTROL_PIN 4     // Pin do sterowania zasilaniem peryferiów
#define DHT_PIN 6               // Nowy pin dla czujnika DHT11
#define LED_CLK_PIN 8           // CLK pin dla wyświetlacza LED
#define LED_DIO_PIN 9           // DIO pin dla wyświetlacza LED

#define DHT_TYPE DHT11          // Typ czujnika DHT11

#define LCD_ADDRESS 0x27        // Adres wyświetlacza LCD
#define LCD_COLS 20             // Liczba kolumn wyświetlacza
#define LCD_ROWS 4              // Liczba wierszy wyświetlacza


//Inicjalizacja modułów

Clock clock;                   // RTC
DhtSensor dhtSensor(DHT_PIN, DHT_TYPE);          // DHT11
WeatherSensor sensor;          //BME 280

LcdDisplay lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);  // LCD
LedDisplay led(LED_CLK_PIN, LED_DIO_PIN);         // LED

// Czujnik dotykowy jest aktywny stanem wysokim (nie ma zworek do zmiany logiki).
DebouncedButton touchSensor(WAKEUP_INTERRUPT_PIN, ActiveState::ACTIVE_HIGH);

PowerManager powerManager(POWER_CONTROL_PIN, RTC_ALARM_PIN, WAKEUP_INTERRUPT_PIN);
CommandHandler commandHandler(clock);



Timer sensorUpdateTimer(1000); 
Timer ledUpdateTimer(500);      
Timer heartbeatTimer(5000);
Timer builtinLedTimer(1000); // Timer do mrugania wbudowaną diodą LED

String g_dateStr, g_timeForLcd, g_timeForLed;
float g_temp_external, g_temp_internal, g_humidity, g_pressure;
float g_temp_dht, g_humidity_dht;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT); // Inicjalizacja wbudowanej diody LED
  Serial.begin(9600);
  Serial.println("\nBooting SmartTent System...");

  if (!clock.init()) {
    Serial.println("Błąd inicjalizacji zegara RTC!");
    //while (1); // Zatrzymanie programu, krytyczny błąd. Odkomentuj w wersji finalnej.
  } else {
    Serial.println("Zegar RTC OK.");
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
      g_temp_external = sensor.getTemperature(); 
      g_humidity = sensor.getHumidity();
      g_pressure = sensor.getPressure();
      g_temp_internal = clock.getTemperature();
      
      dhtSensor.readData();
      g_temp_dht = dhtSensor.getTemperature();
      g_humidity_dht = dhtSensor.getHumidity();

      DateTime now = clock.getTime(); // Pobierz czas tylko raz
      g_dateStr = Clock::formatDate(now);
      g_timeForLcd = Clock::formatTime(now, true);
      g_timeForLed = Clock::formatTime(now, false);

      lcd.update(g_dateStr, g_timeForLcd, g_temp_external, g_humidity, g_temp_internal, g_pressure, g_temp_dht, g_humidity_dht);
    }

    if (ledUpdateTimer.isReady()) { 
      if (g_timeForLed.length() > 0) led.update(g_timeForLed);
    }
    
    if (heartbeatTimer.isReady()) { 
      Serial.println("HEARTBEAT (Aktywny)\n");
      
      Serial.print("\nData:");
      Serial.print(g_dateStr);
      Serial.print("\nCzas:");
      Serial.print(g_timeForLcd);
      Serial.print("\nCzas LED:");
      Serial.print(g_timeForLed);
      Serial.print("\nZewn: ");
      Serial.print(g_temp_external);
      Serial.print("C, Wilg(Z): ");
      Serial.print(g_humidity);
      Serial.print("C, Wewn: ");
      Serial.print(g_temp_internal);
      Serial.print("C, Namiot: ");
      Serial.print(g_temp_dht);
      Serial.print("C, Wilg(N): ");
      Serial.print(g_humidity_dht);
      Serial.print("%, Cisnienir(hPa): ");
      Serial.print(g_pressure);
    }
  }
}