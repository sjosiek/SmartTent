#ifndef SUN_TRACKER_H
#define SUN_TRACKER_H

#include <Arduino.h>
#include "SmoothServo.h"
#include "ControlPanel.h"
#include <limits.h>

// Struktura przechowująca konfigurację pinów
struct SunTrackerPins {
    uint8_t horizontalServoPin;
    uint8_t verticalServoPin;
    uint8_t ldrTopLeftPin;
    uint8_t ldrTopRightPin;
    uint8_t ldrDownLeftPin;
    uint8_t ldrDownRightPin;
    uint8_t joystickXPin;
    uint8_t joystickYPin;
    uint8_t joystickSwPin;
};

// Struktura przechowująca ustawienia działania trackera
struct SunTrackerConfig {
    int servoVMinAngle;
    int servoVMaxAngle;
    int servoHMinAngle;
    int servoHMaxAngle;

    bool performLdrCalibration;
    bool performServoCalibration;
    bool performInitialSearch;
    bool useJoystick;
    bool usePotentiometers;
    bool ldrSensorsConnected;
    bool enableServoMovement;

    int defaultServoSpeed;
    int defaultTolerance;
    uint32_t runningUpdateIntervalMs;
};

class SunTracker {
public:
    SunTracker(const SunTrackerPins& pins, const SunTrackerConfig& config);
    void begin();
    void update();

private:
    // --- Maszyna Stanów Programu ---
    enum class ProgramState {
        STARTUP_WAIT,
        INIT,
        LDR_CALIBRATE_START,
        LDR_CALIBRATE_PROMPT,
        LDR_CALIBRATE_WAIT,
        SERVO_CALIBRATE_HORIZONTAL,
        SERVO_CALIBRATE_VERTICAL,
        CENTERING,
        SEARCHING,
        RUNNING,
        PARKED,
        MANUAL_CONTROL
    };

    // --- Struktury danych ---
    enum class LdrPin : uint8_t {
        TopLeft = 0, TopRight = 1, DownLeft = 2, DownRight = 3
    };
    static constexpr int LDR_COUNT = 4;

    struct LdrCalibrationData {
        uint16_t magicKey;
        int ldrMin[LDR_COUNT];
        int ldrMax[LDR_COUNT];
    };

    // --- Metody pomocnicze ---
    void saveCalibrationData();
    bool loadCalibrationData();
    void printCalibrationData();
    int normalizeLDR(int rawValue, int ldrIndex);
    void handleStateMachine();
    void printDebugInfo();

    // --- Obiekty i konfiguracja ---
    SunTrackerPins pins;
    SunTrackerConfig config;

    SmoothServo horizontalServo;
    SmoothServo verticalServo;
    ControlPanel controlPanel;

    // --- Zmienne stanu ---
    ProgramState currentState;
    uint32_t startupEntryTime;
    uint32_t lastRunningUpdateTime;
    uint32_t lastDebugPrintTime;
    uint32_t searchWaitStartTime;
    long lastMeasuredLightIntensity;

    // --- Dane kalibracyjne LDR ---
    int ldrMin[LDR_COUNT];
    int ldrMax[LDR_COUNT];
    bool calibrationDataLoaded;
    int calibrationLdrIndex;
    bool isCalibratingLight;
    const char* ldrNames[LDR_COUNT] = {"Górny-Lewy", "Górny-Prawy", "Dolny-Lewy", "Dolny-Prawy"};

    // --- Zmienne dla trybu wyszukiwania słońca ---
    static constexpr int SEARCH_STEP = 20;
    int searchHorizontalAngle;
    int searchVerticalAngle;
    long bestLightIntensity;
    int bestHorizontalAngle;
    int bestVerticalAngle;
    
    static constexpr uint32_t SEARCH_WAIT_TIME = 250;

    // --- Zmienne do debugowania ---
    
    static constexpr uint32_t DEBUG_PRINT_INTERVAL = 500;

    // --- Zmienne tymczasowe dla pętli ---
    int topLeftVal, topRightVal, downLeftVal, downRightVal;
    int verticalDiff, horizontalDiff;

    // --- Konfiguracja EEPROM ---
    static constexpr int EEPROM_ADDR = 0;
    static constexpr uint16_t EEPROM_MAGIC_KEY = 0x5453; // "TS" for "Tracker Settings"
};

#endif // SUN_TRACKER_H