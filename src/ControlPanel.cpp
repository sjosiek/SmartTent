#include "ControlPanel.h"

ControlPanel::ControlPanel(const ModulePins& pins) :
  _pins(pins),
  _encoder(pins.encDT, pins.encCLK),
  // ZMIANA: Inicjalizujemy obiekty DebouncedButton odpowiednimi pinami.
  // Domyślnie używają one logiki ACTIVE_LOW, co jest poprawne dla INPUT_PULLUP.
  _joy1Button(pins.joy1Btn),
  _joy2Button(pins.joy2Btn),
  _encButton(pins.encBtn)
{}

// ZMIANA: begin() zapisuje ustawienia martwego pola
void ControlPanel::begin(int joyCenter, int joyDeadZone) {
  _joyCenter = joyCenter;
  _joyDeadZone = joyDeadZone;
  pinMode(_pins.buzzer, OUTPUT);
}

// update() odczytuje teraz tylko surowe wartości
void ControlPanel::update() {
  _joy1X = analogRead(_pins.joy1X);
  _joy1Y = analogRead(_pins.joy1Y);
  _joy2X = analogRead(_pins.joy2X);
  _joy2Y = analogRead(_pins.joy2Y);

  _encoderValue = _encoder.read();

  // ZMIANA: Aktualizujemy stan wszystkich przycisków za pomocą ich dedykowanych obiektów.
  _joy1Button.update();
  _joy2Button.update();
  _encButton.update();
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
bool ControlPanel::isJoy1Pressed() { return _joy1Button.isPressed(); }
bool ControlPanel::wasJoy1Clicked() { return _joy1Button.wasPressed(); }

// Analogicznie dla Joysticka 2
int ControlPanel::getJoy2XRaw() { return _joy2X; }
int ControlPanel::getJoy2YRaw() { return _joy2Y; }
int ControlPanel::getJoy2XMapped() { return _applyDeadZoneAndMap(_joy2X); }
int ControlPanel::getJoy2YMapped() { return _applyDeadZoneAndMap(_joy2Y); }
JoyDirection ControlPanel::getJoy2Direction() { return _getDirection(_joy2X, _joy2Y); }
bool ControlPanel::isJoy2Pressed() { return _joy2Button.isPressed(); }
bool ControlPanel::wasJoy2Clicked() { return _joy2Button.wasPressed(); }


// Reszta metod (encoder, buzzer) bez zmian
long ControlPanel::getEncoderValue() { return _encoderValue / 4; }
void ControlPanel::resetEncoder(long newValue) { _encoder.write(newValue * 4); }
bool ControlPanel::isEncoderPressed() { return _encButton.isPressed(); }
bool ControlPanel::wasEncoderClicked() { return _encButton.wasPressed(); }
void ControlPanel::beep(unsigned int frequency, unsigned long duration) { tone(_pins.buzzer, frequency, duration); }
void ControlPanel::playTone(unsigned int frequency) { tone(_pins.buzzer, frequency); }
void ControlPanel::stopTone() { noTone(_pins.buzzer); }

void ControlPanel::printDebugInfo() {
  Serial.println(F("--- Control Panel Debug ---"));
  
  // Joystick 1
  Serial.print(F("Joy1: Raw(X,Y): "));
  Serial.print(getJoy1XRaw());
  Serial.print(F(", "));
  Serial.print(getJoy1YRaw());
  Serial.print(F(" | Mapped(X,Y): "));
  Serial.print(getJoy1XMapped());
  Serial.print(F(", "));
  Serial.print(getJoy1YMapped());
  Serial.print(F(" | Dir: "));
  Serial.print(getJoy1Direction());
  Serial.print(F(" | Btn: "));
  Serial.println(isJoy1Pressed() ? F("PRESSED") : F("RELEASED"));

  // Enkoder
  Serial.print(F("Encoder: Value: "));
  Serial.print(getEncoderValue());
  Serial.print(F(" | Btn: "));
  Serial.println(isEncoderPressed() ? F("PRESSED") : F("RELEASED"));
  Serial.println(F("---------------------------"));
}
