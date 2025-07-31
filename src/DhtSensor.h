// Plik: DhtSensor.h

#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include <Arduino.h>
#include <DHT.h>

class DhtSensor {
public:
  DhtSensor(uint8_t pin, uint8_t type);
  void init();
  void readData();
  float getTemperature();
  float getHumidity();

private:
  DHT dht;
  float temperature;
  float humidity;
  unsigned long lastReadTime;
};

#endif