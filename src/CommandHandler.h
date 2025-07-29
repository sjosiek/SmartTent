// Plik: CommandHandler.h

#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "Clock.h"

class CommandHandler {
public:
  CommandHandler(Clock& clock);
  void update();

private:
  Clock* _clock;
};

#endif