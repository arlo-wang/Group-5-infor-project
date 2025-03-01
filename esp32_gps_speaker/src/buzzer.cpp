#include "../include/buzzer.hpp"
#include "../include/gps.hpp"

namespace buzzer {
    constexpr int buzzer_PIN = BUZZER_PIN;
    BuzzerState currentState = IDLE;
    unsigned long lastBeepTime = 0;
    unsigned long alarmStartTime = 0;
    int alarmSequenceStep = 0;
    
    // Buzzer setup
    void localSetup() {
        pinMode(buzzer_PIN, OUTPUT);
        digitalWrite(buzzer_PIN, LOW);  // Ensure buzzer is initially off
    }
    
    void beep(int frequency, int duration) {
        tone(buzzer_PIN, frequency, duration);
    }
    
    void triggerAlarm(BuzzerState alarmType) {
        if (currentState == IDLE) {
            currentState = alarmType;
            alarmStartTime = millis();
            alarmSequenceStep = 0;
            Serial.print("Alarm triggered: ");
            Serial.println(alarmType);
        }
    }
    
    // Buzzer loop
    void localLoop() {
        // Check GPS geofence alarm
        if (gps::isGeofenceAlarmTriggered() && currentState == IDLE) {
            triggerAlarm(GEOFENCE_ALARM);
        }
        
        // Declare variables outside the switch statement
        unsigned long currentTime;
        unsigned long elapsedTime;
        
        // Control buzzer based on current state
        switch (currentState) {
            case GEOFENCE_ALARM:
                // Geofence alarm pattern: alternating high-low tones
                currentTime = millis();
                elapsedTime = currentTime - lastBeepTime;
                
                if (elapsedTime >= 500) {
                    lastBeepTime = currentTime;
                    
                    // Alternate between high and low frequency tones
                    if (alarmSequenceStep % 2 == 0) {
                        beep(4000, 500);  // High frequency (4000Hz)
                    } else {
                        beep(2000, 500);  // Low frequency (2000Hz)
                    }
                    
                    alarmSequenceStep++;
                    
                    // After 6 steps (3 high-low pairs), reset if needed
                    if (alarmSequenceStep >= 6) {
                        alarmSequenceStep = 0;
                    }
                }
                break;
                
            case BLUETOOTH_ALARM:
                // Bluetooth alarm pattern: rapid beeps
                currentTime = millis();
                elapsedTime = currentTime - lastBeepTime;
                
                if (elapsedTime >= 200) {
                    lastBeepTime = currentTime;
                    
                    if (alarmSequenceStep % 2 == 0) {
                        beep(3000, 200);  // Medium frequency (3000Hz)
                    } else {
                        noTone(buzzer_PIN);
                    }
                    
                    alarmSequenceStep++;
                    
                    // After 10 steps (5 beeps), reset
                    if (alarmSequenceStep >= 10) {
                        alarmSequenceStep = 0;
                    }
                }
                break;
                
            case IDLE:
            default:
                // Do nothing in idle state
                noTone(buzzer_PIN);
                break;
        }
    }
    
    // Stop alarm
    void stopAlarm() {
        currentState = IDLE;
        noTone(buzzer_PIN);
        
        // Reset GPS geofence alarm if that was the source
        if (currentState == GEOFENCE_ALARM) {
            gps::resetGeofenceAlarm();
        }
    }
    
    void stopAlarmIfType(BuzzerState alarmType) {
        if (currentState == alarmType) {
            stopAlarm();
        }
    }
} // namespace buzzer 