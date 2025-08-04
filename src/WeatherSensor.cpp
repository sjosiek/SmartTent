// Plik: WeatherSensor.cpp

#include "WeatherSensor.h"

WeatherSensor::WeatherSensor(uint8_t address) 
  : _address(address), _isInitialized(false) {
  // Zerowanie wartości początkowych
  temperature = 0.0f;
  humidity = 0.0f;
  pressure = 0.0f;
}

bool WeatherSensor::init() {
  // Pierwsza próba inicjalizacji
  _isInitialized = bme.begin(_address);
  return _isInitialized;
}

void WeatherSensor::readData() {
  if (!_isInitialized) {
    // Jeśli czujnik nie jest zainicjalizowany (np. po utracie zasilania),
    // spróbuj go zainicjalizować ponownie.
    _isInitialized = bme.begin(_address);
    if (!_isInitialized) {
      // Jeśli inicjalizacja się nie powiodła, zeruj dane i wyjdź.
      temperature = 0.0f;
      humidity = 0.0f;
      pressure = 0.0f;
      return;
    }
  }

  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;

  // Jeśli którykolwiek odczyt to NaN, oznacza to błąd komunikacji.
  // Oznaczamy czujnik jako wymagający re-inicjalizacji.
  if (isnan(temperature) || isnan(humidity) || isnan(pressure)) {
    Serial.println("Błąd odczytu z BME280, utracono połączenie.");
    _isInitialized = false;
    // Zerujemy dane, aby nie wyświetlać starych, nieaktualnych wartości.
    temperature = 0.0f;
    humidity = 0.0f;
    pressure = 0.0f;
  }
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