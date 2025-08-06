// Plik: SmoothServo.cpp
#include "SmoothServo.h"

// Definicja sekwencji kalibracji
struct CalibrationStep {
    int position;
    uint32_t wait_ms;
};

static const CalibrationStep calibrationSequence[] = {
    {0, 2000}, {45, 1000}, {90, 1000}, {135, 1000}, {180, 2000},
    {135, 1000}, {90, 1000}, {45, 1000}, {0, 1000}
};
static const int calibrationStepCount = sizeof(calibrationSequence) / sizeof(calibrationSequence[0]);


SmoothServo::SmoothServo() 
    : pin(0), name("Uninitialized"), currentPos(90), targetPos(90), minAngle(0), maxAngle(180),
      state(ServoState::IDLE), calibrationStep(0), calibrationWaitStartTime(0), stepSize(1), moveDelay(20), lastMoveTime(0) {}

SmoothServo::SmoothServo(uint8_t pin, const char* name)
    : pin(pin),
      name(name),
      currentPos(90),
      targetPos(90),
      minAngle(0),
      maxAngle(180),
      state(ServoState::IDLE),
      calibrationStep(0),
      calibrationWaitStartTime(0),
      stepSize(1),
      moveDelay(20),
      lastMoveTime(0) {}

void SmoothServo::begin(uint8_t pin, const char* name, int startPos, int minAngle, int maxAngle) {
    this->pin = pin;
    servo.attach(pin);
    this->name = name;
    this->minAngle = minAngle;
    this->maxAngle = maxAngle;
    currentPos = constrain(startPos, this->minAngle, this->maxAngle);
    targetPos = currentPos;
    servo.write(currentPos);
}

void SmoothServo::setTargetPosition(int target) {
    state = ServoState::MOVING;
    targetPos = constrain(target, minAngle, maxAngle);
}

void SmoothServo::setSpeed(uint16_t moveDelayMs) {
    moveDelay = moveDelayMs;
}

void SmoothServo::setStepSize(uint8_t stepSize) {
    this->stepSize = stepSize;
}

bool SmoothServo::update() {
    // Część 1: Logika maszyny stanów (decyduje, co robić dalej)
    if (state == ServoState::CALIBRATING) {
        if (hasReachedTarget()) { // Jeśli nie poruszamy się do celu kalibracji...
            // ...oznacza to, że dotarliśmy do celu i teraz czekamy.
            if (calibrationWaitStartTime == 0) {
                calibrationWaitStartTime = millis(); // Uruchom stoper oczekiwania
            }

            if (millis() - calibrationWaitStartTime >= calibrationSequence[calibrationStep].wait_ms) {
                // Czas oczekiwania minął, przechodzimy do następnego kroku.
                calibrationStep++;
                calibrationWaitStartTime = 0; // Resetuj stoper

                if (calibrationStep >= calibrationStepCount) {
                    // Koniec kalibracji
                    state = ServoState::IDLE;
                } else {
                    // Ustaw następny cel z sekwencji
                    targetPos = constrain(calibrationSequence[calibrationStep].position, minAngle, maxAngle);
                }
            }
        }
    } else if (state == ServoState::MOVING) {
        if (hasReachedTarget()) {
            state = ServoState::IDLE;
        }
    }

    // Część 2: Logika fizycznego ruchu (wykonuje się zawsze, gdy nie jesteśmy w celu)
    if (currentPos != targetPos) {
        if (millis() - lastMoveTime >= moveDelay) {
            lastMoveTime = millis();
            if (targetPos > currentPos) {
                currentPos = min(currentPos + stepSize, targetPos);
            } else {
                currentPos = max(currentPos - stepSize, targetPos);
            }
            servo.write(currentPos);
        }
    }

    return state != ServoState::IDLE; // Zwraca true, jeśli serwo jest zajęte (w ruchu lub kalibracji)
}

void SmoothServo::write(int position) {
    currentPos = constrain(position, minAngle, maxAngle);
    targetPos = currentPos;
    servo.write(currentPos);
}

int SmoothServo::getCurrentPosition() const { return currentPos; }

int SmoothServo::getTargetPosition() const { return targetPos; }

bool SmoothServo::hasReachedTarget() const { return currentPos == targetPos; }

void SmoothServo::moveLeft() {
    setTargetPosition(currentPos - stepSize);
}

void SmoothServo::moveRight() {
    setTargetPosition(currentPos + stepSize);
}

void SmoothServo::startCalibration() {
    state = ServoState::CALIBRATING;
    calibrationStep = 0;
    calibrationWaitStartTime = 0;
    targetPos = constrain(calibrationSequence[0].position, minAngle, maxAngle);
    Serial.println("Rozpoczynam kalibrację serwa...");
}

bool SmoothServo::isCalibrating() const { return state == ServoState::CALIBRATING; }