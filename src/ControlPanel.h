#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <Arduino.h>
#include <Encoder.h>

// NOWOŚĆ: Enum do reprezentowania kierunków joysticka
enum JoyDirection {
  CENTER, UP, DOWN, LEFT, RIGHT,
  UP_LEFT, UP_RIGHT, DOWN_LEFT, DOWN_RIGHT
};

// Struktura z pinami pozostaje bez zmian
struct ModulePins {
  byte joy1X, joy1Y, joy1Btn;
  byte joy2X, joy2Y, joy2Btn;
  byte encDT, encCLK, encBtn;
  byte buzzer;
};

class ControlPanel {
public:
  ControlPanel(const ModulePins& pins);

  // ZMIANA: begin() przyjmuje teraz konfigurację martwego pola
  void begin(int joyCenter = 512, int joyDeadZone = 50);

  void update();

  // ZMIANA: Dodano metody Raw, Mapped i Direction
  int getJoy1XRaw(); // Zwraca surową wartość 0-1023
  int getJoy1YRaw();
  int getJoy1XMapped(); // Zwraca wartość -100 do 100 z martwym polem
  int getJoy1YMapped();
  JoyDirection getJoy1Direction(); // Zwraca kierunek jako enum
  bool isJoy1Pressed();
  bool wasJoy1Clicked();

  // Analogiczne zmiany dla Joysticka 2
  int getJoy2XRaw();
  int getJoy2YRaw();
  int getJoy2XMapped();
  int getJoy2YMapped();
  JoyDirection getJoy2Direction();
  bool isJoy2Pressed();
  bool wasJoy2Clicked();
  
  long getEncoderValue();
  void resetEncoder(long newValue = 0);
  bool isEncoderPressed();
  bool wasEncoderClicked();

  void beep(unsigned int frequency, unsigned long duration);
  void playTone(unsigned int frequency);
  void stopTone();

  // Metoda do debugowania
  void printDebugInfo();

private:
  // NOWOŚĆ: Prywatne metody do logiki joysticka
  int _applyDeadZoneAndMap(int value);
  JoyDirection _getDirection(int x, int y);
  
  ModulePins _pins;
  Encoder _encoder;
  long _encoderValue;
  int _joy1X, _joy1Y, _joy2X, _joy2Y;
  bool _joy1BtnState, _joy2BtnState, _encBtnState;
  bool _lastJoy1BtnState, _lastJoy2BtnState, _lastEncBtnState;
  bool _joy1Clicked, _joy2Clicked, _encClicked;

  // NOWOŚĆ: Zmienne do obsługi martwego pola
  int _joyCenter;
  int _joyDeadZone;
};
#endif