// Plik: CommandHandler.cpp

#include "CommandHandler.h"


CommandHandler::CommandHandler(Clock& clock, SDCard& sdCard, Configuration& config, SmoothServo* servos, int servoCount) 
  : _clock(&clock), _sdCard(&sdCard), _config(&config), _servos(servos), _servoCount(servoCount) {}

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
    } else if (command == "SAVE_CONFIG") {
      Serial.println("Rozpoznano komendę zapisu konfiguracji.");
      if (_sdCard->writeConfiguration(*_config, "config.txt")) {
        Serial.println("OK: Konfiguracja zapisana.");
      } else {
        Serial.println("BŁĄD: Nie udało się zapisać konfiguracji.");
      }
    } else if (command.startsWith("SERVO:")) {
      // Format komendy: SERVO:<index>:<pozycja>, np. "SERVO:0:90"
      int firstColon = command.indexOf(':');
      int secondColon = command.indexOf(':', firstColon + 1);

      if (firstColon != -1 && secondColon != -1) {
        int servoIndex = command.substring(firstColon + 1, secondColon).toInt();
        int position = command.substring(secondColon + 1).toInt();

        if (servoIndex >= 0 && servoIndex < _servoCount) {
          Serial.print("Ustawiam pozycję docelową dla SERVO");
          Serial.print(servoIndex);
          Serial.print(" na: ");
          Serial.println(position);
          _servos[servoIndex].setTargetPosition(position);
        } else {
          Serial.println("BŁĄD: Nieprawidłowy indeks serwa.");
        }
      }
    } else if (command.startsWith("SERVO_MOVE:")) {
      // Format: SERVO_MOVE:<index>:<LEFT|RIGHT>
      int firstColon = command.indexOf(':');
      int secondColon = command.indexOf(':', firstColon + 1);

      if (firstColon != -1 && secondColon != -1) {
          int servoIndex = command.substring(firstColon + 1, secondColon).toInt();
          String direction = command.substring(secondColon + 1);
          direction.toUpperCase();

          if (servoIndex >= 0 && servoIndex < _servoCount) {
              if (direction == "LEFT") {
                  _servos[servoIndex].moveLeft();
              } else if (direction == "RIGHT") {
                  _servos[servoIndex].moveRight();
              } else {
                  Serial.println("BŁĄD: Nieprawidłowy kierunek (użyj LEFT lub RIGHT).");
              }
          } else {
              Serial.println("BŁĄD: Nieprawidłowy indeks serwa.");
          }
      }
    } else if (command == "CALIBRATE_SERVOS") {
      for (int i = 0; i < _servoCount; i++) {
        _servos[i].startCalibration();
      }
    }
  }
}