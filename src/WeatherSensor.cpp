// Plik: WeatherSensor.cpp

#include "WeatherSensor.h"

WeatherSensor::WeatherSensor() {}

bool WeatherSensor::init() {
  return bme.begin(0x76);
}

void WeatherSensor::readData() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
}

float WeatherSensor::getTemperature() {
  return temperature;
}

float WeatherSensor::getHumidity() {
  return humidity;
}

float WeatherSensor::getPressure() {
  return pressure;
}