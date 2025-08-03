#ifndef SMOOTH_SERVO_H
#define SMOOTH_SERVO_H

#include <Servo.h>
#include <Arduino.h>

class SmoothServo {
public:
    // Konstruktor
    SmoothServo(uint8_t pin, uint8_t ledL, uint8_t ledR, String sName = "Servo", int minP = 0, int maxP = 180, uint8_t stepVal = 1, uint16_t delayMs = 20);

    // Metody publiczne
    void begin(int startPos = 90);
    void update();
    
    void setPosition(int pos);
    int getPosition();
    
    void calibrate();

    void attachEncoder(int clk, int dt);
    void attachJoystick(int pin, bool setAnalog = false);
    void attachButtons(int leftPin, int rightPin);

private:
    // Metody prywatne
    // void updateEncoder();
    // void updateJoystick();
    // void updateButtons();
    // void updateEffects();
    void moveLeft();
    void moveRight();
    void controlAnalog(int val, int jPin);
    // void blinkLED(uint8_t pin);
    // void flashLED(uint8_t pin);
    // void turnOnLED(uint8_t pin);
    // void buzz();

    // Zmienne członkowskie
    Servo servo;
    uint8_t pin;
    int pos;
    int minPos, maxPos;
    uint8_t step;
    uint32_t lastMoveTime;
    uint16_t moveDelay;
    uint8_t ledLeft, ledRight;
    String servoName;
    
    int encoderClkPin, encoderDtPin, lastClk;
    bool encoderAttached;

    int joystickPin;
    bool joystickAttached, setPositionFromAnalog;

    int btnLeftPin, btnRightPin;
    bool buttonsAttached;
};

#endif // SMOOTH_SERVO_H

