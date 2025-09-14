// Plik: SDCard.h

#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>
#include "SensorData.h"
#include "Configuration.h"

class SDCard {
public:
  SDCard(uint8_t csPin);
  bool init();
  bool isOK() const; // Zwraca false, jeśli wystąpił błąd zapisu (np. karta pełna)

  // Metoda do logowania danych z czujników do określonego pliku
  void logSensorData(const SensorData& data, const char* filename);
  
  // Metoda do odczytu pliku konfiguracyjnego
  bool readConfiguration(const char* filename, Configuration& config);

  // Metoda do zapisu aktualnej konfiguracji do pliku
  bool writeConfiguration(const Configuration& config, const char* filename);

private:
  uint8_t _csPin;
  bool _isInitialized;
  bool _writeErrorOccurred;
};

#endif