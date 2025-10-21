// Plik: CommandHandler.h

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include <string.h> // ZMIANA: Dodajemy dla funkcji strcasecmp
#include "Clock.h"
#include "LcdDisplay.h" // Dołączamy LcdDisplay
#include "SDCard.h"
#include "ProjectConfig.h"
#include "SmoothServo.h"

class CommandHandler {
public:
  CommandHandler(Clock& clock, LcdDisplay& lcd, SDCard& sdCard, Configuration& config, SmoothServo* servos, int servoCount);
  void update();

private:
  Clock* _clock;
  LcdDisplay* _lcd;
  SDCard* _sdCard;
  Configuration* _config;
  SmoothServo* _servos;
  int _servoCount;
};

#endif