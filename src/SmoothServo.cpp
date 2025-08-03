#include <Servo.h>
#include <Arduino.h>

class SmoothServo {
  Servo servo;
  uint8_t pin;
  int pos;
  int minPos, maxPos;
  uint8_t step;
  uint32_t lastMoveTime;
  uint16_t moveDelay;
  uint8_t ledLeft, ledRight;
  String servoName;
  

  // Enkoder
  int encoderClkPin = -1, encoderDtPin = -1;
  int lastClk = HIGH;
  bool encoderAttached = false;

  // Joystick
  int joystickPin = -1;
  bool joystickAttached = false;
  bool setPositionFromAnalog = false;

  // Przyciski
  int btnLeftPin = -1, btnRightPin = -1;
  bool buttonsAttached = false;

  // LED/Buzzer stany czasowe
  uint8_t activeLED = 255;
  uint8_t flashCount = 0;
  uint32_t flashStartTime = 0;
  bool flashing = false;

  bool buzzing = false;
  uint32_t buzzStartTime = 0;

public: 
  SmoothServo(uint8_t pin, uint8_t ledL, uint8_t ledR, String sName = "Servo", int minP = 0, int maxP = 90, uint8_t stepVal = 2, uint16_t delayMs = 100)
    : pin(pin), minPos(minP), maxPos(maxP), step(stepVal), moveDelay(delayMs), ledLeft(ledL), ledRight(ledR) {
    pos = 0;
    lastMoveTime = 0;
    servoName = sName;
  }

  void setPosition(int pos = 0)
  {
    // if (DEBUG_ENABLED) Serial.print("Set "), Serial.print(servoName), Serial.print(" position: "), Serial.println(pos) ;
    servo.write(pos);
  }

  int getPostion()
  {
    // if (DEBUG_ENABLED) Serial.print(servoName), Serial.print(" position: "), Serial.println(pos) ;
    return pos;
  }

  void calibrate()
  {
    // if (DEBUG_ENABLED) Serial.print("Calibrating: "), Serial.print(servoName), Serial.print(": "), Serial.println(pos) ;
    delay(2000);
    setPosition(0);
    delay(3000);
    setPosition(45);
    delay(2000);
    setPosition(90);
    delay(2000);
    setPosition(135);
    delay(2000);
    setPosition(180);
    delay(2000);
    setPosition(135);
    delay(2000);
    setPosition(90);
    delay(2000);
    setPosition(45);
    delay(2000);
    setPosition(0);
  }

//   void begin(int startPos = 0) {
//     servo.attach(pin);
//     pos = constrain(startPos, minPos, maxPos);
//     // if (DEBUG_ENABLED) Serial.print("Start position for -> "), Serial.print(servoName), Serial.print(": "), Serial.println(pos) ;
//     servo.write(pos);
//     pinMode(ledLeft, OUTPUT);
//     pinMode(ledRight, OUTPUT);
//     digitalWrite(ledLeft, LOW);
//     digitalWrite(ledRight, LOW);
//   }

  

//   void attachEncoder(int clk, int dt) {
//     encoderClkPin = clk;
//     encoderDtPin = dt;
//     pinMode(encoderClkPin, INPUT_PULLUP);
//     pinMode(encoderDtPin, INPUT_PULLUP);
//     encoderAttached = true;
//   }

//   void attachJoystick(int pin, bool setAnalog = false) {
//     joystickPin = pin;
//     joystickAttached = true;
//     setPositionFromAnalog = setAnalog;
//   }

//   void attachButtons(int leftPin, int rightPin) {
//   btnLeftPin = leftPin;
//   btnRightPin = rightPin;
//   pinMode(btnLeftPin, INPUT_PULLUP);
//   pinMode(btnRightPin, INPUT_PULLUP);
//   buttonsAttached = true;
// }

//   void update() {
//     updateEncoder();
//     updateJoystick();
//     updateButtons();
//     updateEffects();
//   }

//   void updateEncoder() {
//     if (!encoderAttached) return;
//     int clkState = digitalRead(encoderClkPin);
//     if (clkState != lastClk && clkState == LOW) {
//       int dtState = digitalRead(encoderDtPin);
//       if (dtState != clkState) {
//         // if (DEBUG_ENABLED) Serial.println("Encoder: right");
//         moveRight();
//       } else {
//         // if (DEBUG_ENABLED) Serial.println("Encoder: left");
//         moveLeft();
//       }
//     }
//     lastClk = clkState;
//   }

//   void updateJoystick() {
//     if (!joystickAttached) return;
//     int val = analogRead(joystickPin);
//    // if (DEBUG_ENABLED) Serial.print("Joystick: "), Serial.println(val);

//     if (setPositionFromAnalog) {
//       int newPos = map(val, 0, 1023, minPos, maxPos);
//       if (abs(newPos - pos) >= step) {
//         pos = newPos;
//         servo.write(pos);
//         // if (DEBUG_ENABLED) Serial.print("Set pos from analog: "), Serial.println(pos);
//       }
//     } else {
//       controlAnalog(val, joystickPin);
//     }
//   }

//   void updateButtons() {
//   if (!buttonsAttached) return;

//   bool leftPressed = digitalRead(btnLeftPin) == LOW;
//   bool rightPressed = digitalRead(btnRightPin) == LOW;

//   if (leftPressed) {
//     // if (DEBUG_ENABLED) Serial.println("Button: left");
//     moveLeft();
//   }

//   if (rightPressed) {
//     // if (DEBUG_ENABLED) Serial.println("Button: right");
//     moveRight();
//   }
// }

  void moveLeft() {
    if (millis() - lastMoveTime < moveDelay) return;
    if (pos > minPos) {
      pos -= step;
      pos = max(pos, minPos);
      servo.write(pos);
    //   blinkLED(ledLeft);
      // if (DEBUG_ENABLED) Serial.print("Move Left. Position: "), Serial.println(pos);
    } else {
    //   flashLED(ledLeft);
      //turnOnLED(ledLeft);
    //   buzz();
      // if (DEBUG_ENABLED) Serial.println("Limit reached on Left");
    }
    lastMoveTime = millis();
  }

  void moveRight() {
    if (millis() - lastMoveTime < moveDelay) return;
    if (pos < maxPos) {
      pos += step;
      pos = min(pos, maxPos);
      servo.write(pos);
      //blinkLED(ledRight);
      // if (DEBUG_ENABLED) Serial.print("Move Right. Position: "), Serial.println(pos);
    } else {
     // flashLED(ledRight);
      //turnOnLED(ledRight);
     // buzz();
      // if (DEBUG_ENABLED) Serial.println("Limit reached on Right");
    }
    lastMoveTime = millis();
  }

//   void controlAnalog(int val, int jPin) {
//     //if (DEBUG_ENABLED) Serial.print("controlAnalog: "), Serial.println(jPin);
//     //delay(1000);
//     if (val < 450) moveRight();
//     else if (val > 574) moveLeft();
//   }

//   void blinkLED(uint8_t pin) {
//     // if (DEBUG_ENABLED) Serial.println("Blink Led"), Serial.println(pin);
//     digitalWrite(pin, HIGH);
//     activeLED = pin;
//     flashStartTime = millis();
//     flashCount = 1;
//     flashing = true;
//   }

//   void flashLED(uint8_t pin) {
//     // if (DEBUG_ENABLED) Serial.println("Flash Led"), Serial.println(pin);
//     activeLED = pin;
//     flashStartTime = millis();
//     flashCount = 0;
//     flashing = true;
//   }

//   void turnOnLED(uint8_t pin) {
//   digitalWrite(pin, HIGH);
//   flashing = false;
//   activeLED = 255;
//   }

// void buzz() {
//   // Grają różne dźwięki przy osiągnięciu limitu
//   if (pos <= minPos) {
//     // Niski ton przy osiągnięciu limitu lewego
//     tone(BUZZER_PIN, 600, 200);  // 600 Hz przez 200ms
//   } else if (pos >= maxPos) {
//     // Wysoki ton przy osiągnięciu limitu prawego
//     tone(BUZZER_PIN, 1800, 200); // 1800 Hz przez 200ms
//   } else {
//     // Ton ostrzegawczy przy zmianie pozycji
//     tone(BUZZER_PIN, 1200, 100); // 1200 Hz przez 100ms
//   }
  
//   buzzStartTime = millis();
//   buzzing = true;
// }

//   void updateEffects() {
//     if (flashing && millis() - flashStartTime >= 50) {
//       digitalWrite(activeLED, !digitalRead(activeLED));
//       flashStartTime = millis();
//       flashCount++;
//       if (flashCount >= 4) {
//         flashing = false;
//         digitalWrite(activeLED, LOW);
//       }
//     }

//     if (buzzing && millis() - buzzStartTime >= 200) {
//       digitalWrite(BUZZER_PIN, LOW);
//       buzzing = false;
//     }
//   }
};