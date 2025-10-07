// Plik: HardwareConfigReader.cpp

#include "HardwareConfigReader.h"

HardwareConfigReader::HardwareConfigReader(uint8_t latchPin, uint8_t clockPin, uint8_t dataPin)
    : _latchPin(latchPin), _clockPin(clockPin), _dataPin(dataPin) {}

void HardwareConfigReader::begin() {
  pinMode(_latchPin, OUTPUT);
  pinMode(_clockPin, OUTPUT);
  pinMode(_dataPin, INPUT);
  digitalWrite(_latchPin, HIGH);
}

uint16_t HardwareConfigReader::readSwitches() {
  uint16_t bits = 0;

  // Krok 1: Zatrzaskujemy stan 16 przełączników do wewnętrznych rejestrów 74HC165
  digitalWrite(_latchPin, LOW);
  delayMicroseconds(5); // Krótka pauza
  digitalWrite(_latchPin, HIGH);

  // Krok 2: Odczytujemy szeregowo 16 bitów
  for (int i = 0; i < 16; i++) {
    bits <<= 1; // Przesuń bity w lewo, robiąc miejsce na nowy bit
    if (digitalRead(_dataPin) == HIGH) {
      bits |= 1; // Ustaw najmłodszy bit, jeśli na wejściu jest stan wysoki
    }
    // Krok 3: Wygeneruj impuls zegarowy, aby przesunąć następny bit na wyjście
    digitalWrite(_clockPin, HIGH);
    delayMicroseconds(5);
    digitalWrite(_clockPin, LOW);
  }
  return bits;
}