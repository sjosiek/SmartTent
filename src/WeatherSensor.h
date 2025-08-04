// Plik: WeatherSensor.h

#ifndef WEATHERSENSOR_H
#define WEATHERSENSOR_H

#include <Adafruit_BME280.h>

class WeatherSensor {
public:
  WeatherSensor(uint8_t address = 0x76);
  bool init();
  void readData();
  float getTemperature();
  float getHumidity();
  float getPressure();

private:
  Adafruit_BME280 bme;
  uint8_t _address;
  bool _isInitialized;
  float temperature;
  float humidity;
  float pressure;
};

#endif