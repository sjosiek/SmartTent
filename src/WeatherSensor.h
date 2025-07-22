// Plik: WeatherSensor.h

#ifndef WEATHERSENSOR_H
#define WEATHERSENSOR_H

#include <Adafruit_BME280.h>

class WeatherSensor {
public:
  WeatherSensor();
  bool init();
  void readData();
  float getTemperature();
  float getHumidity();
  float getPressure();

private:
  Adafruit_BME280 bme;
  float temperature;
  float humidity;
  float pressure;
};

#endif