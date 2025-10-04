#include "SunTracker.h"
#include <EEPROM.h>

SunTracker::SunTracker(const SunTrackerPins& pins, const SunTrackerConfig& config, ControlPanel& controlPanel)
    : pins(pins),
      config(config),
      controlPanel(controlPanel), // ZMIANA: Inicjalizujemy referencję
      currentState(ProgramState::STARTUP_WAIT),
      startupEntryTime(0),
      lastRunningUpdateTime(0),
      lastDebugPrintTime(0),
      searchWaitStartTime(0),
      lastMeasuredLightIntensity(0),
      // ldrMin, ldrMax - inicjalizowane w ciele konstruktora
      calibrationDataLoaded(false),
      calibrationLdrIndex(0),
      isCalibratingLight(true),
      // ldrNames - inicjalizowany w pliku .h
      searchHorizontalAngle(config.servoHMinAngle),
      searchVerticalAngle(config.servoVMinAngle),
      bestLightIntensity(-1),
      bestHorizontalAngle(90),
      bestVerticalAngle(30)
      
      
      
{
    // Domyślne wartości kalibracji
    for (int i = 0; i < LDR_COUNT; ++i) {
        ldrMin[i] = 100;
        ldrMax[i] = 900;
    }
}

void SunTracker::begin() {
    horizontalServo.begin(pins.horizontalServoPin, "Horizontal", 90, config.servoHMinAngle, config.servoHMaxAngle);
    verticalServo.begin(pins.verticalServoPin, "Vertical", 90, config.servoVMinAngle, config.servoVMaxAngle);
    horizontalServo.setStepSize(5);
    verticalServo.setStepSize(5);

    Serial.println(F("\n--- Sun Tracker v2.0 Library ---"));
    startupEntryTime = millis();
}

void SunTracker::update() {
    horizontalServo.update();
    verticalServo.update();

    if (config.useJoystick) {
        controlPanel.update();
        if (controlPanel.wasJoy1Clicked()) {
            if (currentState == ProgramState::RUNNING || currentState == ProgramState::PARKED) {
                currentState = ProgramState::MANUAL_CONTROL;
                Serial.println(F("\nPrzełączono na sterowanie ręczne (joystick)."));
            } else if (currentState == ProgramState::MANUAL_CONTROL) {
                currentState = ProgramState::RUNNING;
                lastRunningUpdateTime = 0; // Zresetuj timer, aby wymusić aktualizację
                Serial.println(F("\nPrzełączono na tryb automatyczny."));
            }
        }
    }

    handleStateMachine();
    if (config.enableDebugPrint) {
        printDebugInfo();
    }
}

void SunTracker::handleStateMachine() {
    switch (currentState) {
        case ProgramState::STARTUP_WAIT:
            if (millis() - startupEntryTime >= 3000) {
                calibrationDataLoaded = loadCalibrationData();
                currentState = ProgramState::INIT;
            }
            break;

        case ProgramState::INIT:
            if (config.ldrSensorsConnected && (config.performLdrCalibration || !calibrationDataLoaded)) {
                currentState = ProgramState::LDR_CALIBRATE_START;
            } else {
                if (!config.ldrSensorsConnected) {
                    Serial.println(F("\nFotorezystory wyłączone. Tracker uruchomi się w trybie bezczynności."));
                } else {
                    Serial.println(F("\nPomijam kalibrację LDR. Używam wartości z pamięci lub domyślnych."));
                }
                if (config.performServoCalibration) {
                    Serial.println(F("Rozpoczynam kalibrację serwomechanizmów..."));
                    horizontalServo.startCalibration();
                    currentState = ProgramState::SERVO_CALIBRATE_HORIZONTAL;
                } else {
                    Serial.println(F("Pomijam kalibrację serwomechanizmów. Ustawiam pozycję startową..."));
                    horizontalServo.setTargetPosition(bestHorizontalAngle);
                    verticalServo.setTargetPosition(90);
                    currentState = ProgramState::CENTERING;
                }
            }
            break;

        case ProgramState::LDR_CALIBRATE_START:
            for (int i = 0; i < LDR_COUNT; i++) { ldrMin[i] = 1023; ldrMax[i] = 0; }
            calibrationLdrIndex = 0;
            isCalibratingLight = true;
            Serial.println(F("\n--- Rozpoczynam indywidualną kalibrację LDR ---"));
            currentState = ProgramState::LDR_CALIBRATE_PROMPT;
            break;

        case ProgramState::LDR_CALIBRATE_PROMPT:
            Serial.println();
            Serial.print(F("Krok ")); Serial.print(calibrationLdrIndex * 2 + (isCalibratingLight ? 1 : 2)); Serial.print(F("/8: "));
            Serial.print(F("Kalibracja czujnika '")); Serial.print(ldrNames[calibrationLdrIndex]); Serial.println(F("'"));

            if (isCalibratingLight) {
                Serial.println(F("OŚWIETL go maksymalnie i wyślij dowolny znak, aby zapisać wartość."));
            } else {
                Serial.println(F("ZASŁOŃ go całkowicie i wyślij dowolny znak, aby zapisać wartość."));
            }
            currentState = ProgramState::LDR_CALIBRATE_WAIT;
            break;

        case ProgramState::LDR_CALIBRATE_WAIT:
        {
            uint8_t ldrPinAddresses[] = {pins.ldrTopLeftPin, pins.ldrTopRightPin, pins.ldrDownLeftPin, pins.ldrDownRightPin};
            int currentValue = analogRead(ldrPinAddresses[calibrationLdrIndex]);
            if (isCalibratingLight) {
                ldrMax[calibrationLdrIndex] = max(ldrMax[calibrationLdrIndex], currentValue);
            } else {
                ldrMin[calibrationLdrIndex] = min(ldrMin[calibrationLdrIndex], currentValue);
            }
        }

        if (Serial.available() > 0) {
            Serial.read(); // Wyczyść bufor

            if (isCalibratingLight) {
                isCalibratingLight = false;
                currentState = ProgramState::LDR_CALIBRATE_PROMPT;
            } else {
                calibrationLdrIndex++;
                isCalibratingLight = true;

                if (calibrationLdrIndex < LDR_COUNT) {
                    currentState = ProgramState::LDR_CALIBRATE_PROMPT;
                } else {
                    Serial.println(F("\nKalibracja LDR zakończona."));
                    saveCalibrationData();
                    printCalibrationData();
                    if (config.performServoCalibration) {
                        Serial.println(F("\nRozpoczynam kalibrację serwomechanizmów..."));
                        horizontalServo.startCalibration();
                        currentState = ProgramState::SERVO_CALIBRATE_HORIZONTAL;
                    } else {
                        Serial.println(F("\nPomijam kalibrację serwomechanizmów. Ustawiam pozycję startową..."));
                        horizontalServo.setTargetPosition(90);
                        verticalServo.setTargetPosition(90);
                        currentState = ProgramState::CENTERING;
                    }
                }
            }
        }
        break;

        case ProgramState::SERVO_CALIBRATE_HORIZONTAL:
            if (!horizontalServo.isCalibrating()) {
                Serial.println(F("Kalibracja serwa poziomego zakończona."));
                Serial.println(F("Rozpoczynam kalibrację serwa pionowego..."));
                verticalServo.startCalibration();
                currentState = ProgramState::SERVO_CALIBRATE_VERTICAL;
            }
            break;

        case ProgramState::SERVO_CALIBRATE_VERTICAL:
            if (!verticalServo.isCalibrating()) {
                Serial.println(F("Kalibracja serwa pionowego zakończona. Ustawiam pozycję startową (H:90 V:90 stopni)..."));
                horizontalServo.setTargetPosition(bestHorizontalAngle);
                verticalServo.setTargetPosition(90);
                currentState = ProgramState::CENTERING;
            }
            break;

        case ProgramState::CENTERING:
            if (horizontalServo.hasReachedTarget() && verticalServo.hasReachedTarget()) {
                if (config.ldrSensorsConnected) {
                    if (config.performInitialSearch) {
                        Serial.println(F("Pozycja startowa osiągnięta. Rozpoczynam wyszukiwanie słońca..."));
                        currentState = ProgramState::SEARCHING;
                    } else {
                        Serial.println(F("Pozycja startowa osiągnięta. Pomijam wyszukiwanie, rozpoczynam śledzenie."));
                        lastRunningUpdateTime = 0; // Zresetuj timer, aby wymusić aktualizację
                        currentState = ProgramState::RUNNING;
                    }
                } else {
                    Serial.println(F("Pozycja spoczynkowa osiągnięta. Tracker w trybie bezczynności."));
                    currentState = ProgramState::PARKED;
                }
            }
            break;

        case ProgramState::SEARCHING:
            // Ustawienie prędkości serwomechanizmów na domyślną prędkość z konfiguracji.
            // Spowoduje to, że ruchy podczas wyszukiwania będą tak samo płynne/wolne
            // jak podczas trybu RUNNING.
            horizontalServo.setSpeed(config.defaultServoSpeed);
            verticalServo.setSpeed(config.defaultServoSpeed);

            if (horizontalServo.hasReachedTarget() && verticalServo.hasReachedTarget()) {
                if (searchWaitStartTime == 0) {
                    searchWaitStartTime = millis();
                }

                if (millis() - searchWaitStartTime >= SEARCH_WAIT_TIME) {
                    long currentLightIntensity = 0;
                    uint8_t ldrPinAddresses[] = {pins.ldrTopLeftPin, pins.ldrTopRightPin, pins.ldrDownLeftPin, pins.ldrDownRightPin};
                    for (int i = 0; i < LDR_COUNT; i++) {
                        currentLightIntensity += analogRead(ldrPinAddresses[i]);
                    }
                    lastMeasuredLightIntensity = currentLightIntensity;

                    if (currentLightIntensity > bestLightIntensity) {
                        bestLightIntensity = currentLightIntensity;
                        bestHorizontalAngle = searchHorizontalAngle;
                        bestVerticalAngle = searchVerticalAngle;
                    }

                    searchHorizontalAngle += SEARCH_STEP;
                    if (searchHorizontalAngle > config.servoHMaxAngle) {
                        searchHorizontalAngle = config.servoHMinAngle;
                        searchVerticalAngle += SEARCH_STEP;
                    }

                    if (searchVerticalAngle > config.servoVMaxAngle) {
                        Serial.print(F("Wyszukiwanie zakończone. Najjaśniejszy punkt przy (H,V): "));
                        Serial.print(bestHorizontalAngle); Serial.print(F(",")); Serial.println(bestVerticalAngle);
                        horizontalServo.setTargetPosition(bestHorizontalAngle);
                        verticalServo.setTargetPosition(bestVerticalAngle);
                        lastRunningUpdateTime = 0; // Zresetuj timer, aby wymusić aktualizację po dotarciu do celu
                        currentState = ProgramState::RUNNING;
                    } else {
                        horizontalServo.setTargetPosition(searchHorizontalAngle);
                        verticalServo.setTargetPosition(searchVerticalAngle);
                        searchWaitStartTime = 0;
                    }
                }
            }
            break;

        case ProgramState::RUNNING:
        {
            // Upewnij się, że serwa są na miejscu, zanim zaczniesz odliczać czas
            if (horizontalServo.hasReachedTarget() && verticalServo.hasReachedTarget()) {
                // Jeśli to pierwsze wejście po dotarciu na miejsce, uruchom timer
                if (lastRunningUpdateTime == 0) {
                    lastRunningUpdateTime = millis();
                    Serial.println(F("Cel osiągnięty. Rozpoczynam cykliczne śledzenie..."));
                }

                // Czekaj na upłynięcie interwału aktualizacji
                if (millis() - lastRunningUpdateTime >= config.runningUpdateIntervalMs) {
                    lastRunningUpdateTime = millis(); // Zresetuj timer na następny interwał
                    Serial.println(F("Odczytuję dane z LDR"));

                    int raw_tl = analogRead(pins.ldrTopLeftPin);
                    int raw_tr = analogRead(pins.ldrTopRightPin);
                    int raw_dl = analogRead(pins.ldrDownLeftPin);
                    int raw_dr = analogRead(pins.ldrDownRightPin);

                    topLeftVal   = normalizeLDR(raw_tl, static_cast<int>(LdrPin::TopLeft));
                    topRightVal  = normalizeLDR(raw_tr, static_cast<int>(LdrPin::TopRight));
                    downLeftVal  = normalizeLDR(raw_dl, static_cast<int>(LdrPin::DownLeft));
                    downRightVal = normalizeLDR(raw_dr, static_cast<int>(LdrPin::DownRight));

                    int servoMoveSpeed = config.defaultServoSpeed;
                    int tolerance = config.defaultTolerance;

                    horizontalServo.setSpeed(servoMoveSpeed);
                    verticalServo.setSpeed(servoMoveSpeed);

                    int sumTop = topLeftVal + topRightVal;
                    int sumDown = downLeftVal + downRightVal;
                    int sumLeft = topLeftVal + downLeftVal;
                    int sumRight = topRightVal + downRightVal;

                    verticalDiff   = sumTop - sumDown;
                    horizontalDiff = sumLeft - sumRight;
                    
                    if (abs(verticalDiff) > tolerance) {
                        if (sumTop > sumDown) {
                            if (config.enableServoMovement) verticalServo.reverseMoveUp();
                        } else {
                            if (config.enableServoMovement) verticalServo.reverseMoveDown();
                        }
                    }
                    if (abs(horizontalDiff) > tolerance) {
                        if (sumLeft > sumRight) {
                            if (config.enableServoMovement) horizontalServo.reverseMoveLeft();
                        } else {
                            if (config.enableServoMovement) horizontalServo.reverseMoveRight();
                        }
                    }

                    // ZMIANA: Przenosimy logowanie informacji o stanie RUNNING tutaj.
                    // Dzięki temu komunikat pojawi się tylko raz, po wykonaniu aktualizacji (co 5 minut),
                    // a nie co 500ms, jak to było w funkcji printDebugInfo().
                    if (config.enableDebugPrint) {
                        Serial.print(F("  -> LDR(TL,TR,DL,DR): "));
                        Serial.print(topLeftVal); Serial.print(F(",")); Serial.print(topRightVal); Serial.print(F(","));
                        Serial.print(downLeftVal); Serial.print(F(",")); Serial.println(downRightVal);
                        Serial.print(F("  -> Diffs(H,V): ")); Serial.print(horizontalDiff); Serial.print(F(",")); Serial.println(verticalDiff);
                    }
                }
            }
        }
        break;

        case ProgramState::PARKED:
            break;

        case ProgramState::MANUAL_CONTROL:
        {
            if (!config.useJoystick) {
                currentState = ProgramState::PARKED;
                break;
            }

            JoyDirection dir = controlPanel.getJoy1Direction();

            switch(dir) {
                case UP:    verticalServo.moveUp(); break;
                case DOWN:  verticalServo.moveDown(); break;
                case LEFT:  horizontalServo.moveLeft(); break;
                case RIGHT: horizontalServo.moveRight(); break;
                default: break;
            }
        }
        break;

        default:
            Serial.println(F("BŁĄD: Nieznany lub nieobsługiwany stan programu!"));
            break;
    }
}

void SunTracker::printDebugInfo() {
    if (millis() - lastDebugPrintTime >= DEBUG_PRINT_INTERVAL) {
        lastDebugPrintTime = millis();

        switch (currentState) {
            case ProgramState::STARTUP_WAIT:
                Serial.println(F("Uruchamianie... Proszę czekać."));
                break;
            case ProgramState::INIT:
                Serial.println(F("Inicjalizacja..."));
                break;
            case ProgramState::LDR_CALIBRATE_START:
            case ProgramState::LDR_CALIBRATE_PROMPT:
            case ProgramState::LDR_CALIBRATE_WAIT:
            {
                uint8_t ldrPinAddresses[] = {pins.ldrTopLeftPin, pins.ldrTopRightPin, pins.ldrDownLeftPin, pins.ldrDownRightPin};
                int currentValue = analogRead(ldrPinAddresses[calibrationLdrIndex]);
                Serial.print(F("Kalibracja '")); Serial.print(ldrNames[calibrationLdrIndex]);
                Serial.print(F("' (")); Serial.print(isCalibratingLight ? F("Jasno") : F("Ciemno"));
                Serial.print(F("). Aktualny odczyt: ")); Serial.println(currentValue);
            }
            break;
            case ProgramState::SERVO_CALIBRATE_HORIZONTAL:
                Serial.print(F("Kalibracja serwa poziomego... H: "));
                Serial.print(horizontalServo.getCurrentPosition());
                Serial.print(F(" -> "));
                Serial.println(horizontalServo.getTargetPosition());
                break;
            case ProgramState::SERVO_CALIBRATE_VERTICAL:
                Serial.print(F("Kalibracja serwa pionowego... V: "));
                Serial.print(verticalServo.getCurrentPosition());
                Serial.print(F(" -> "));
                Serial.println(verticalServo.getTargetPosition());
                break;
            case ProgramState::CENTERING:
                Serial.print(F("Centrowanie... H: "));
                Serial.print(horizontalServo.getCurrentPosition());
                Serial.print(F("->"));
                Serial.print(horizontalServo.getTargetPosition());
                Serial.print(F(" V: "));
                Serial.print(verticalServo.getCurrentPosition());
                Serial.print(F("->"));
                Serial.println(verticalServo.getTargetPosition());
                break;
            case ProgramState::SEARCHING:
                Serial.print(F("Wyszukiwanie... Cel (H,V): "));
                Serial.print(horizontalServo.getTargetPosition()); Serial.print(F(",")); Serial.print(verticalServo.getTargetPosition());
                Serial.print(F(" | Aktualna (H,V): "));
                Serial.print(horizontalServo.getCurrentPosition()); Serial.print(F(",")); Serial.print(verticalServo.getCurrentPosition());
                Serial.print(F(" | Najlepsza: "));
                Serial.print(bestHorizontalAngle); Serial.print(F(",")); Serial.println(bestVerticalAngle);
                Serial.print(F("  -> Pomiar: ")); Serial.print(lastMeasuredLightIntensity);
                Serial.print(F(" | Najlepszy pomiar: ")); Serial.println(bestLightIntensity);
                break;
            case ProgramState::RUNNING:
                // ZMIANA: W stanie RUNNING nie drukujemy już nic w tej funkcji.
                // Logowanie zostało przeniesione do bloku `case ProgramState::RUNNING` w `handleStateMachine`,
                // aby pojawiało się tylko w momencie faktycznej aktualizacji pozycji.
                // Można tu dodać logikę oczekiwania, jeśli chcesz.
                break;
            case ProgramState::PARKED:
                Serial.print(F("Zaparkowany. Pozycja (H,V): "));
                Serial.print(horizontalServo.getCurrentPosition());
                Serial.print(F(","));
                Serial.println(verticalServo.getCurrentPosition());
                break;
            case ProgramState::MANUAL_CONTROL:
                Serial.print(F("Sterowanie ręczne. Kierunek: ")); Serial.print(controlPanel.getJoy1Direction());
                Serial.print(F(" | Pozycja (H,V): "));
                Serial.print(horizontalServo.getCurrentPosition());
                Serial.print(F(","));
                Serial.println(verticalServo.getCurrentPosition());
                break;
        }
    }
}

void SunTracker::saveCalibrationData() {
    LdrCalibrationData data;
    data.magicKey = EEPROM_MAGIC_KEY;
    for (int i = 0; i < LDR_COUNT; i++) {
        data.ldrMin[i] = ldrMin[i];
        data.ldrMax[i] = ldrMax[i];
    }
    EEPROM.put(EEPROM_ADDR, data);
    Serial.println(F("Dane kalibracyjne LDR zapisane w EEPROM."));
}

void SunTracker::printCalibrationData() {
    Serial.println(F("Aktualne dane kalibracyjne LDR:"));
    for (int i = 0; i < LDR_COUNT; i++) {
        Serial.print(F("  LDR ")); Serial.print(i); Serial.print(F(" (")); Serial.print(ldrNames[i]); Serial.print(F("): "));
        Serial.print(ldrMin[i]); Serial.print(F(" - ")); Serial.println(ldrMax[i]);
    }
}

bool SunTracker::loadCalibrationData() {
    LdrCalibrationData data;
    EEPROM.get(EEPROM_ADDR, data);

    if (data.magicKey == EEPROM_MAGIC_KEY) {
        for (int i = 0; i < LDR_COUNT; i++) {
            ldrMin[i] = data.ldrMin[i];
            ldrMax[i] = data.ldrMax[i];
        }
        Serial.println(F("Pomyślnie załadowano dane kalibracyjne LDR z EEPROM."));
        printCalibrationData();
        return true;
    } else {
        Serial.println(F("Brak poprawnych danych kalibracyjnych w EEPROM."));
        return false;
    }
}

int SunTracker::normalizeLDR(int rawValue, int ldrIndex) {
    if (ldrMax[ldrIndex] <= ldrMin[ldrIndex]) {
        return 0;
    }
    int constrainedVal = constrain(rawValue, ldrMin[ldrIndex], ldrMax[ldrIndex]);
    return map(constrainedVal, ldrMin[ldrIndex], ldrMax[ldrIndex], 0, 1000);
}

// --- Metody dostępowe (gettery) ---

int SunTracker::getHorizontalServoPosition() const {
    return horizontalServo.getCurrentPosition();
}

int SunTracker::getVerticalServoPosition() const {
    return verticalServo.getCurrentPosition();
}

void SunTracker::getLdrValues(int& tl, int& tr, int& dl, int& dr) const {
    tl = topLeftVal;  tr = topRightVal;
    dl = downLeftVal; dr = downRightVal;
}