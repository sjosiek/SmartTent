// Plik: DhtSensor.cpp

#include "DhtSensor.h"

DhtSensor::DhtSensor(uint8_t pin, uint8_t type) : dht(pin, type) {
  temperature = 0.0f;
  humidity = 0.0f;
  lastReadTime = 0;
}

void DhtSensor::init() {
  dht.begin();
}

void DhtSensor::readData() {
  // Czujniki DHT nie powinny być odczytywane częściej niż co 2 sekundy.
  if (millis() - lastReadTime < 2000) {
    return;
  }
  lastReadTime = millis();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Sprawdzamy, czy odczyty są prawidłowymi liczbami.
  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
  }
}

float DhtSensor::getTemperature() { return temperature; }

float DhtSensor::getHumidity() { return humidity; }