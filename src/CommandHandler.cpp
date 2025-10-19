// Plik: CommandHandler.cpp

#include "CommandHandler.h"


CommandHandler::CommandHandler(Clock& clock, LcdDisplay& lcd, SDCard& sdCard, Configuration& config, SmoothServo* servos, int servoCount) 
  : _clock(&clock), _lcd(&lcd), _sdCard(&sdCard), _config(&config), _servos(servos), _servoCount(servoCount) {}

void CommandHandler::update() {
  if (Serial.available() > 0) {
    // ZMIANA: Zastępujemy String buforem char[], aby uniknąć alokacji pamięci.
    char command[64]; // Bufor na komendę, 64 znaki powinny wystarczyć.
    int bytesRead = Serial.readBytesUntil('\n', command, sizeof(command) - 1);
    command[bytesRead] = '\0'; // Ręcznie dodajemy terminator null.

    // Ręczny "trim" - usuwamy znaki powrotu karetki z końca, jeśli istnieją.
    if (bytesRead > 0 && command[bytesRead - 1] == '\r') {
      command[bytesRead - 1] = '\0';
    }

    // ZMIANA: Wyświetl komunikat na LCD
    char buffer[21];
    snprintf(buffer, sizeof(buffer), "CMD: %s", command);
    _lcd->showTemporaryMessage("Odebrano komende:", buffer, 5000);

    Serial.print(F("Odebrano z portu szeregowego: \""));
    Serial.print(command);
    Serial.println(F("\""));

    if (strncmp(command, "TIME:", 5) == 0) {
      int year, month, day, hour, minute, second;
      // Używamy sscanf do parsowania formatu: TIME:YYYY-MM-DD,HH:MM:SS
      if (sscanf(command, "TIME:%d-%d-%d,%d:%d:%d", &year, &month, &day, &hour, &minute, &second) == 6) {
        Serial.println(F("Rozpoznano i poprawnie sparsowano komendę synchronizacji czasu."));
        _clock->adjust(DateTime(year, month, day, hour, minute, second));

        // ZMIANA: Używamy bufora zamiast String do wysłania potwierdzenia.
        char confirmation[40];
        snprintf(confirmation, sizeof(confirmation), "OK: Czas zsynchronizowany do %s", command + 5);
        Serial.println(confirmation);
      } else {
        Serial.println(F("BŁĄD: Nieprawidłowy format komendy TIME. Oczekiwano: TIME:YYYY-MM-DD,HH:MM:SS"));
      }
    } else if (strcmp(command, "SAVE_CONFIG") == 0) {
      Serial.println(F("Rozpoznano komendę zapisu konfiguracji."));
      if (_sdCard->writeConfiguration(*_config, "config.txt")) {
        Serial.println(F("OK: Konfiguracja zapisana."));
      } else {
        Serial.println(F("BŁĄD: Nie udało się zapisać konfiguracji na karcie SD."));
      }
    } else if (strncmp(command, "SERVO:", 6) == 0) {
      int servoIndex, position;
      // Używamy sscanf do parsowania formatu: SERVO:<index>:<pozycja>
      if (sscanf(command, "SERVO:%d:%d", &servoIndex, &position) == 2) {
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
    } else if (strncmp(command, "SERVO_MOVE:", 11) == 0) {
      int servoIndex;
      char directionBuffer[10]; // Bufor na "LEFT" lub "RIGHT"
      // Używamy sscanf do parsowania formatu: SERVO_MOVE:<index>:<kierunek>
      if (sscanf(command, "SERVO_MOVE:%d:%s", &servoIndex, directionBuffer) == 2) {
          if (servoIndex >= 0 && servoIndex < _servoCount) {
              // ZMIANA: Używamy strcasecmp do porównywania stringów bez względu na wielkość liter.
              if (strcasecmp(directionBuffer, "LEFT") == 0) {
                  _servos[servoIndex].moveLeft();
              } else if (strcasecmp(directionBuffer, "RIGHT") == 0) {
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
    } else if (strcmp(command, "CALIBRATE_SERVOS") == 0) {
      for (int i = 0; i < _servoCount; i++) {
        _servos[i].startCalibration();
      }
    }
  }
}