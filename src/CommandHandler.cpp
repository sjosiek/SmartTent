// Plik: CommandHandler.cpp

#include "CommandHandler.h"

CommandHandler::CommandHandler(Clock& clock) : _clock(&clock) {}

void CommandHandler::update() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    
    Serial.println("Odebrano z portu szeregowego: \"" + command + "\"");

    if (command.startsWith("TIME:")) {
      Serial.println("Rozpoznano komendę synchronizacji czasu.");
      
      // Oczekiwany format: TIME:YYYY-MM-DD,HH:MM:SS
      String payload = command.substring(5);

      // Parsowanie danych
      int year = payload.substring(0, 4).toInt();
      int month = payload.substring(5, 7).toInt();
      int day = payload.substring(8, 10).toInt();
      int hour = payload.substring(11, 13).toInt();
      int minute = payload.substring(14, 16).toInt();
      int second = payload.substring(17, 19).toInt();

      // Ustaw czas w RTC, używając wskaźnika do obiektu Clock
      _clock->adjust(DateTime(year, month, day, hour, minute, second));

      String confirmation = "OK: Czas zsynchronizowany do " + payload;
      Serial.println(confirmation);
    }
  }
}