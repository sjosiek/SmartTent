// Plik: SDCard.h

#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>
#include "SensorData.h"

class SDCard {
public:
  SDCard(uint8_t csPin);
  bool init();

  // Metoda do logowania danych z czujników do określonego pliku
  void logSensorData(const SensorData& data, const char* filename);

  // W przyszłości można tu dodać inne metody, np. do odczytu plików konfiguracyjnych

private:
  uint8_t _csPin;
  bool _isInitialized;
};

#endif