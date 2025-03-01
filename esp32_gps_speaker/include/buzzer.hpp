#pragma once

#include <Arduino.h>
#include "../include/gps.hpp"

#define BUZZER_PIN 39  // Buzzer pin

namespace buzzer {
    // Buzzer states
    enum BuzzerState {
        IDLE,
        GEOFENCE_ALARM,
        BLUETOOTH_ALARM,
        // Other alarm states...
    };
    
    // External declarations
    extern BuzzerState currentState;
    extern unsigned long lastBeepTime;
    extern unsigned long alarmStartTime;
    extern int alarmSequenceStep;
    
    // Function declarations
    void localSetup();
    void beep(int frequency, int duration);
    void localLoop();
    void stopAlarm();
    void stopAlarmIfType(BuzzerState alarmType);
    void triggerAlarm(BuzzerState alarmType);
} // namespace buzzer