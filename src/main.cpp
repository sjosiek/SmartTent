// Plik główny: main.cpp
// Wersja z poprawioną logiką w funkcji setup()

#include <Arduino.h>
#include "WeatherSensor.h"
#include "Clock.h"
#include "LcdDisplay.h"
#include "LedDisplay.h"
#include "Timer.h"
#include "PowerManager.h"

const bool SLEEP_MODE_ENABLED = false;

#define POWER_CONTROL_PIN 4
#define WAKEUP_INTERRUPT_PIN 2

WeatherSensor sensor;
Clock clock;
LcdDisplay lcd(0x27, 20, 4);
LedDisplay led(2, 3);
PowerManager powerManager(POWER_CONTROL_PIN, WAKEUP_INTERRUPT_PIN, LogicLevel::ACTIVE_HIGH);

Timer sensorUpdateTimer(1000); 
Timer ledUpdateTimer(500);      
Timer heartbeatTimer(5000);

String g_dateStr, g_timeForLcd, g_timeForLed;
float g_temp_external, g_temp_internal, g_humidity, g_pressure;

void setup() {
  Serial.begin(9600);
  Serial.println("\nBooting SmartTent System...");

  clock.init(); 
  
  if (SLEEP_MODE_ENABLED) {
    Serial.println("Tryb oszczędzania energii WŁĄCZONY.");
    powerManager.begin(clock, lcd);
  } else {
    Serial.println("Tryb oszczędzania energii WYŁĄCZONY. System będzie działał w trybie ciągłym.");
    pinMode(POWER_CONTROL_PIN, OUTPUT);
    
    // ZMIANA: Używamy teraz publicznej metody z PowerManagera do włączenia zasilania
    powerManager.powerUpPeripherals();
    
    // Inicjalizujemy resztę modułów
    sensor.init();
    lcd.init();
    led.init(10);
    lcd.printWelcomeMessage();
    delay(2000);
  }
}

void loop() {
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
      g_dateStr = clock.getDateString();
      g_timeForLcd = clock.getTimeString(true);
      g_timeForLed = clock.getTimeString(false);
      lcd.update(g_dateStr, g_timeForLcd, g_temp_external, g_temp_internal, g_humidity, g_pressure);
    }

    if (ledUpdateTimer.isReady()) { 
      if (g_timeForLed.length() > 0) led.update(g_timeForLed);
    }
    
    if (heartbeatTimer.isReady()) { 
      Serial.print("HEARTBEAT (Aktywny) -> Zewn: ");
      Serial.print(g_temp_external);
      Serial.print("C, Wewn: ");
      Serial.print(g_temp_internal);
      Serial.println("C");
    }
  }
}