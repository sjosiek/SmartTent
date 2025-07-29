// Plik główny: main.cpp
// Wersja z poprawioną logiką w funkcji setup()

#include <Arduino.h>
#include "WeatherSensor.h"
#include "Clock.h"
#include "LcdDisplay.h"
#include "LedDisplay.h"
#include "Timer.h"
#include "PowerManager.h"
#include "CommandHandler.h"

const bool SLEEP_MODE_ENABLED = false;

#define POWER_CONTROL_PIN 4
#define WAKEUP_INTERRUPT_PIN 2

WeatherSensor sensor;
Clock clock;
LcdDisplay lcd(0x27, 20, 4);
LedDisplay led(2, 3);
PowerManager powerManager(POWER_CONTROL_PIN, WAKEUP_INTERRUPT_PIN, LogicLevel::ACTIVE_HIGH);
CommandHandler commandHandler(clock);

Timer sensorUpdateTimer(1000); 
Timer ledUpdateTimer(500);      
Timer heartbeatTimer(5000);
Timer builtinLedTimer(1000); // Timer do mrugania wbudowaną diodą LED

String g_dateStr, g_timeForLcd, g_timeForLed;
float g_temp_external, g_temp_internal, g_humidity, g_pressure;

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
    if (clock.rtc.lostPower()) {
      Serial.println("RTC stracił zasilanie! Ustawiam czas na czas kompilacji.");
      // Poniższa linia ustawi czas na datę i godzinę kompilacji tego szkicu
      clock.rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
  }
    
  if (SLEEP_MODE_ENABLED) {
    Serial.println("Tryb oszczędzania energii WŁĄCZONY.");
    powerManager.begin(clock, lcd, led, sensor);
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
    
    if (SLEEP_MODE_ENABLED && digitalRead(WAKEUP_INTERRUPT_PIN) == LOW) {
      powerManager.resetActiveTimer();
      delay(200);
    }
    
    if (sensorUpdateTimer.isReady()) { 
      sensor.readData(); 
      g_temp_external = sensor.getTemperature(); 
      g_humidity = sensor.getHumidity();
      g_pressure = sensor.getPressure();
      g_temp_internal = clock.getTemperature();

      DateTime now = clock.getTime(); // Pobierz czas tylko raz
      g_dateStr = Clock::formatDate(now);
      g_timeForLcd = Clock::formatTime(now, true);
      g_timeForLed = Clock::formatTime(now, false);

      lcd.update(g_dateStr, g_timeForLcd, g_temp_external, g_temp_internal, g_humidity, g_pressure);
    }

    if (ledUpdateTimer.isReady()) { 
      if (g_timeForLed.length() > 0) led.update(g_timeForLed);
    }
    
    if (heartbeatTimer.isReady()) { 
      Serial.print("HEARTBEAT (Aktywny)\n");
      
      Serial.print("\nData:");
      Serial.print(g_dateStr);
      Serial.print("\nCzas:");
      Serial.print(g_timeForLcd);
      Serial.print("\nCzas LED:");
      Serial.print(g_timeForLed);
      Serial.print("\nZewn: ");
      Serial.print(g_temp_external);
      Serial.print("C, Wewn: ");
      Serial.print(g_temp_internal);
      Serial.println("C");
    }
  }
}