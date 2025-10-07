// Plik: CommandHandler.cpp

#include "CommandHandler.h"


CommandHandler::CommandHandler(Clock& clock, SDCard& sdCard, Configuration& config, SmoothServo* servos, int servoCount) 
  : _clock(&clock), _sdCard(&sdCard), _config(&config), _servos(servos), _servoCount(servoCount) {}

void CommandHandler::update() {
  if (Serial.available() > 0) {
    String command = Serial.readStringUntil('\n');
    command.trim();

    Serial.print(F("Odebrano z portu szeregowego: \""));
    Serial.print(command);
    Serial.println(F("\""));

    if (command.startsWith("TIME:")) {
      int year, month, day, hour, minute, second;
      // Używamy sscanf do parsowania formatu: TIME:YYYY-MM-DD,HH:MM:SS
      if (sscanf(command.c_str(), "TIME:%d-%d-%d,%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
        Serial.println(F("Rozpoznano i poprawnie sparsowano komendę synchronizacji czasu."));
        _clock->adjust(DateTime(year, month, day, hour, minute, second));

        String confirmation = F("OK: Czas zsynchronizowany do ");
        confirmation += command.substring(5); // Wycinamy tylko dane do potwierdzenia
        Serial.println(confirmation);
      } else {
        Serial.println(F("BŁĄD: Nieprawidłowy format komendy TIME. Oczekiwano: TIME:YYYY-MM-DD,HH:MM:SS"));
      }
    } else if (command == "SAVE_CONFIG") {
      Serial.println(F("Rozpoznano komendę zapisu konfiguracji."));
      if (_sdCard->writeConfiguration(*_config, "config.txt")) {
        Serial.println(F("OK: Konfiguracja zapisana."));
      } else {
        Serial.println(F("BŁĄD: Nie udało się zapisać konfiguracji na karcie SD."));
      }
    } else if (command.startsWith("SERVO:")) {
      int servoIndex, position;
      // Używamy sscanf do parsowania formatu: SERVO:<index>:<pozycja>
      if (sscanf(command.c_str(), "SERVO:%d:%d", &servoIndex, &position) == 2) {
        if (servoIndex >= 0 && servoIndex < _servoCount) {
          Serial.print(F("OK: Ustawiam pozycję docelową dla SERVO"));
          Serial.print(servoIndex); Serial.print(F(" na: ")); Serial.println(position);
          _servos[servoIndex].setTargetPosition(position);
        } else {
          Serial.println(F("BŁĄD: Nieprawidłowy indeks serwa."));
        }
      } else {
        Serial.println(F("BŁĄD: Nieprawidłowy format komendy SERVO. Oczekiwano: SERVO:<index>:<pozycja>"));
      }
    } else if (command.startsWith("SERVO_MOVE:")) {
      int servoIndex;
      char directionBuffer[10]; // Bufor na "LEFT" lub "RIGHT"
      // Używamy sscanf do parsowania formatu: SERVO_MOVE:<index>:<kierunek>
      if (sscanf(command.c_str(), "SERVO_MOVE:%d:%s", &servoIndex, directionBuffer) == 2) {
          if (servoIndex >= 0 && servoIndex < _servoCount) {
              String direction(directionBuffer);
              direction.toUpperCase();
              if (direction == "LEFT") {
                  _servos[servoIndex].moveLeft();
              } else if (direction == "RIGHT") {
                  _servos[servoIndex].moveRight();
              } else {
                  Serial.println(F("BŁĄD: Nieprawidłowy kierunek (użyj LEFT lub RIGHT)."));
              }
          } else {
              Serial.println(F("BŁĄD: Nieprawidłowy indeks serwa lub format komendy."));
          }
      } else {
        Serial.println(F("BŁĄD: Nieprawidłowy format komendy SERVO_MOVE. Oczekiwano: SERVO_MOVE:<index>:<LEFT|RIGHT>"));
      }
    } else if (command == "CALIBRATE_SERVOS") {
      for (int i = 0; i < _servoCount; i++) {
        _servos[i].startCalibration();
      }
    }
  }
}