#include "../include/buzzer.hpp"
#include "../include/gps.hpp"
#include "../include/bluetooth.hpp"

namespace buzzer {
    constexpr int buzzer_PIN = BUZZER_PIN;
    BuzzerState current_state = IDLE;
    unsigned long last_beep_time = 0;
    unsigned long alarm_start_time = 0;
    int alarm_sequence_step = 0;
    
    void initLEDC() 
    {
        ledcSetup(LEDC_CHANNEL, 2000, LEDC_RESOLUTION); 
        ledcAttachPin(BUZZER_PIN, LEDC_CHANNEL);
    }

    // Buzzer setup
    void localSetup() 
    {
        initLEDC();
        pinMode(buzzer_PIN, OUTPUT);
        digitalWrite(buzzer_PIN, LOW);  // Ensure buzzer is initially off
    }
    
    void beep(int frequency, int duration) 
    {
        tone(buzzer_PIN, frequency, duration);
    }
    
    void triggerAlarm(BuzzerState alarm_type) 
    {
        if (current_state == IDLE) {
            current_state = alarm_type;
            alarm_start_time = millis();
            alarm_sequence_step = 0;
            Serial.print("Alarm triggered: ");
            delay(2000);
        }
    }
    
    // Buzzer loop
    void localLoop() 
    {
        // Check GPS geofence alarm
        if (gps::isGeofenceAlarmTriggered() && current_state == IDLE) 
        {
            triggerAlarm(ALARM);
        }
        // reset alarm if geofence alarm is not triggered
        else if ((current_state == ALARM && 
                !gps::isGeofenceAlarmTriggered() && 
                gps::hasValidLocation()) || 
                bluetooth::isDeviceFound()) 
        {
            DEBUGSerial.println("Geofence alarm reset - stopping buzzer");
            stopAlarm();
        }
        
        // Declare variables outside the switch statement
        unsigned long current_time;
        unsigned long elapsed_time;
        
        // Control buzzer based on current state
        switch (current_state) 
        {
            case ALARM:
                // Geofence alarm pattern: alternating high-low tones
                current_time = millis();
                elapsed_time = current_time - last_beep_time;
                
                if (elapsed_time >= 500) 
                {
                    last_beep_time = current_time;
                    
                    // Alternate between high and low frequency tones
                    if (alarm_sequence_step % 2 == 0) 
                    {
                        beep(4000, 500);  // High frequency (4000Hz)
                    } 
                    else 
                    {
                        beep(2000, 500);  // Low frequency (2000Hz)
                    }
                    
                    Serial.print("Alarm triggered: ");

                    alarm_sequence_step++;
                    
                    // After 6 steps (3 high-low pairs), reset if needed
                    if (alarm_sequence_step >= 6) {
                        alarm_sequence_step = 0;
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
    void stopAlarm() 
    {
        current_state = IDLE;
        noTone(buzzer_PIN);
        gps::resetGeofenceAlarm();
    }
    
    void stopAlarmIfType(BuzzerState alarm_type) 
    {
        if (current_state == alarm_type) {
            stopAlarm();
        }
    }
} // namespace buzzer 