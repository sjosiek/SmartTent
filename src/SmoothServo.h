#ifndef SMOOTH_SERVO_H
#define SMOOTH_SERVO_H

#include <Servo.h>
#include <Arduino.h>

class SmoothServo {
private:
    enum class ServoState {
        IDLE,
        MOVING,
        CALIBRATING
    };
public:
    SmoothServo(); // Domyślny konstruktor
    // Konstruktor z bardziej opisowymi nazwami parametrów
    SmoothServo(uint8_t pin, const char* name = "Servo");

    // Dołącza serwo, ustawia jego parametry i pozycję startową
    void begin(uint8_t pin, const char* name, int startPos = 90, int minAngle = 0, int maxAngle = 180);

    // Ustawia pozycję docelową, do której serwo ma się przemieścić
    void setTargetPosition(int target);

    // Ustawia prędkość ruchu (opóźnienie między krokami w ms)
    void setSpeed(uint16_t moveDelayMs);

    // Ustawia wielkość każdego kroku ruchu
    void setStepSize(uint8_t stepSize);

    // Ta metoda powinna być wywoływana cyklicznie w głównej pętli loop()
    // Obsługuje stopniowy ruch w kierunku pozycji docelowej.
    // Zwraca true, jeśli serwo jest wciąż w ruchu.
    bool update();

    // Natychmiastowo przesuwa serwo do pozycji (omija płynny ruch)
    void write(int position);

    // Zwraca aktualną pozycję serwa
    int getCurrentPosition() const;

    // Zwraca pozycję docelową serwa
    int getTargetPosition() const;

    // Sprawdza, czy serwo dotarło do celu
    bool hasReachedTarget() const;

    // Zleca ruch o jeden krok w danym kierunku
    void moveLeft();
    void moveRight();
    void moveUp();
    void moveDown();
    // Odwrócone aliasy ruchu
    void reverseMoveLeft();
    void reverseMoveRight();
    void reverseMoveUp();
    void reverseMoveDown();

    // Rozpoczyna nieblokującą sekwencję kalibracji
    void startCalibration();

    // Sprawdza, czy serwo jest w trakcie kalibracji
    bool isCalibrating() const;

private:
    Servo servo;
    uint8_t pin;
    const char* name;

    int currentPos;
    int targetPos;
    int minAngle;
    int maxAngle;

    ServoState state;
    uint8_t calibrationStep;
    uint32_t calibrationWaitStartTime;

    uint8_t stepSize;
    uint32_t lastMoveTime;
    uint16_t moveDelay;
};

#endif // SMOOTH_SERVO_H
