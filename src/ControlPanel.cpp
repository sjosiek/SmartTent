#include "ControlPanel.h"

ControlPanel::ControlPanel(const ModulePins& pins) :
  _pins(pins),
  _encoder(pins.encDT, pins.encCLK)
{
  _lastJoy1BtnState = HIGH;
  _lastJoy2BtnState = HIGH;
  _lastEncBtnState = HIGH;
}

// ZMIANA: begin() zapisuje ustawienia martwego pola
void ControlPanel::begin(int joyCenter, int joyDeadZone) {
  _joyCenter = joyCenter;
  _joyDeadZone = joyDeadZone;
  
  pinMode(_pins.joy1Btn, INPUT_PULLUP);
  pinMode(_pins.joy2Btn, INPUT_PULLUP);
  pinMode(_pins.encBtn, INPUT_PULLUP);
  pinMode(_pins.buzzer, OUTPUT);
}

// update() odczytuje teraz tylko surowe wartości
void ControlPanel::update() {
  _joy1X = analogRead(_pins.joy1X);
  _joy1Y = analogRead(_pins.joy1Y);
  _joy2X = analogRead(_pins.joy2X);
  _joy2Y = analogRead(_pins.joy2Y);

  _encoderValue = _encoder.read();

  // Logika przycisków bez zmian...
  _joy1BtnState = digitalRead(_pins.joy1Btn);
  if (_joy1BtnState == LOW && _lastJoy1BtnState == HIGH) _joy1Clicked = true;
  _lastJoy1BtnState = _joy1BtnState;

  _joy2BtnState = digitalRead(_pins.joy2Btn);
  if (_joy2BtnState == LOW && _lastJoy2BtnState == HIGH) _joy2Clicked = true;
  _lastJoy2BtnState = _joy2BtnState;

  _encBtnState = digitalRead(_pins.encBtn);
  if (_encBtnState == LOW && _lastEncBtnState == HIGH) _encClicked = true;
  _lastEncBtnState = _encBtnState;
}

// --- NOWOŚĆ: Prywatne metody pomocnicze ---
int ControlPanel::_applyDeadZoneAndMap(int value) {
  if (abs(value - _joyCenter) < _joyDeadZone) {
    return 0; // Wartość w martwym polu
  }
  
  // Mapowanie wartości spoza martwego pola na zakres -100 do 100
  if (value < _joyCenter) {
    return map(value, 0, _joyCenter - _joyDeadZone, -100, 0);
  } else {
    return map(value, _joyCenter + _joyDeadZone, 1023, 0, 100);
  }
}

JoyDirection ControlPanel::_getDirection(int x, int y) {
    bool up = (y < _joyCenter - _joyDeadZone);
    bool down = (y > _joyCenter + _joyDeadZone);
    bool left = (x < _joyCenter - _joyDeadZone);
    bool right = (x > _joyCenter + _joyDeadZone);

    if (up && left) return UP_LEFT;
    if (up && right) return UP_RIGHT;
    if (down && left) return DOWN_LEFT;
    if (down && right) return DOWN_RIGHT;
    if (up) return UP;
    if (down) return DOWN;
    if (left) return LEFT;
    if (right) return RIGHT;

    return CENTER;
}

// --- Implementacje getterów (zmienione i nowe) ---

int ControlPanel::getJoy1XRaw() { return _joy1X; }
int ControlPanel::getJoy1YRaw() { return _joy1Y; }
int ControlPanel::getJoy1XMapped() { return _applyDeadZoneAndMap(_joy1X); }
int ControlPanel::getJoy1YMapped() { return _applyDeadZoneAndMap(_joy1Y); }
JoyDirection ControlPanel::getJoy1Direction() { return _getDirection(_joy1X, _joy1Y); }

// ... reszta getterów dla Joy1 bez zmian (isPressed, wasClicked) ...
bool ControlPanel::isJoy1Pressed() { return _joy1BtnState == LOW; }
bool ControlPanel::wasJoy1Clicked() {
  if (_joy1Clicked) { _joy1Clicked = false; return true; }
  return false;
}

// Analogicznie dla Joysticka 2
int ControlPanel::getJoy2XRaw() { return _joy2X; }
int ControlPanel::getJoy2YRaw() { return _joy2Y; }
int ControlPanel::getJoy2XMapped() { return _applyDeadZoneAndMap(_joy2X); }
int ControlPanel::getJoy2YMapped() { return _applyDeadZoneAndMap(_joy2Y); }
JoyDirection ControlPanel::getJoy2Direction() { return _getDirection(_joy2X, _joy2Y); }
bool ControlPanel::isJoy2Pressed() { return _joy2BtnState == LOW; }
bool ControlPanel::wasJoy2Clicked() {
  if (_joy2Clicked) { _joy2Clicked = false; return true; }
  return false;
}


// Reszta metod (encoder, buzzer) bez zmian
long ControlPanel::getEncoderValue() { return _encoderValue / 4; }
void ControlPanel::resetEncoder(long newValue) { _encoder.write(newValue * 4); }
bool ControlPanel::isEncoderPressed() { return _encBtnState == LOW; }
bool ControlPanel::wasEncoderClicked() {
  if (_encClicked) { _encClicked = false; return true; }
  return false;
}
void ControlPanel::beep(unsigned int frequency, unsigned long duration) { tone(_pins.buzzer, frequency, duration); }
void ControlPanel::playTone(unsigned int frequency) { tone(_pins.buzzer, frequency); }
void ControlPanel::stopTone() { noTone(_pins.buzzer); }