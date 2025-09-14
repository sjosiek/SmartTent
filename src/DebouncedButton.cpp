// Plik: DebouncedButton.cpp

#include "DebouncedButton.h"

DebouncedButton::DebouncedButton(uint8_t pin, ActiveState activeState, unsigned long debounceDelay)
    : _pin(pin), _debounceDelay(debounceDelay) {
  if (activeState == ActiveState::ACTIVE_LOW) {
    _activeLevel = LOW;
    pinMode(_pin, INPUT_PULLUP); // Dla przycisków zwieranych do masy
  } else {
    _activeLevel = HIGH;
    pinMode(_pin, INPUT); // Dla czujników aktywnych stanem wysokim
  }

  _lastButtonState = digitalRead(_pin);
  _buttonState = _lastButtonState;
  _lastDebounceTime = 0;
  _pressEvent = false;
}

void DebouncedButton::update() {
  int reading = digitalRead(_pin);

  if (reading != _lastButtonState) {
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > _debounceDelay) {
    if (reading != _buttonState) {
      _buttonState = reading;
      if (_buttonState == _activeLevel) {
        _pressEvent = true; // Zarejestruj zdarzenie naciśnięcia
      }
    }
  }
  _lastButtonState = reading;
}

bool DebouncedButton::wasPressed() {
  if (_pressEvent) {
    _pressEvent = false; // Skonsumuj zdarzenie
    return true;
  }
  return false;
}

bool DebouncedButton::isPressed() { return (_buttonState == _activeLevel); }