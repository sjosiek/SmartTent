// Plik: Configuration.h

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>

struct Configuration {
  uint32_t activeModeMinutes = 5;       // Domyślnie 5 minut
  uint32_t sensorUpdateIntervalMs = 1000; // Domyślnie 1 sekunda
  uint8_t ledBrightness = 7;            // Domyślnie 7
};

#endif