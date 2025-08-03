// Plik: DebouncedButton.h

#ifndef DEBOUNCED_BUTTON_H
#define DEBOUNCED_BUTTON_H

#include <Arduino.h>

// Definiujemy, jaki stan pinu jest uznawany za "aktywny" (wciśnięty)
enum class ActiveState {
  ACTIVE_LOW,  // Przycisk jest wciśnięty, gdy stan jest LOW (np. przycisk do GND)
  ACTIVE_HIGH  // Przycisk jest wciśnięty, gdy stan jest HIGH (np. czujnik dotykowy)
};

class DebouncedButton {
public:
  DebouncedButton(uint8_t pin, ActiveState activeState = ActiveState::ACTIVE_LOW, unsigned long debounceDelay = 50);
  void update();
  bool wasPressed(); // Zwraca true tylko raz po naciśnięciu
  bool isPressed();  // Zwraca true, dopóki przycisk jest wciśnięty

private:
  uint8_t _pin;
  unsigned long _debounceDelay;
  unsigned long _lastDebounceTime;

  int _buttonState;
  int _lastButtonState;
  int _activeLevel; // Przechowuje HIGH lub LOW w zależności od ActiveState
  bool _pressEvent;
};

#endif