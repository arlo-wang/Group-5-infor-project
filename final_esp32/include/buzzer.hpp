#pragma once

#include <Arduino.h>
#include "../include/gps.hpp"

#define BUZZER_PIN 39  // Buzzer pin (connet this pin to the middle pin of the buzzer)
#define LEDC_CHANNEL 0 // LEDC channel to use
#define LEDC_TIMER 0   // LEDC timer to use
#define LEDC_RESOLUTION 8 // LEDC resolution (8-bit: 0-255)

namespace buzzer 
{
    // Buzzer states
    enum BuzzerState 
    {
        IDLE,
        ALARM,
    };
    
    // External declarations
    extern BuzzerState current_state;
    extern unsigned long last_beep_time;
    extern unsigned long alarm_start_time;
    extern int alarm_sequence_step;
    
    // Function declarations
    void localSetup();
    void initLEDC(); // Initialize LEDC for buzzer
    void beep(int frequency, int duration);
    void localLoop();
    void stopAlarm();
    void stopAlarmIfType(BuzzerState alarm_type);
    void triggerAlarm(BuzzerState alarm_type);
} // namespace buzzer