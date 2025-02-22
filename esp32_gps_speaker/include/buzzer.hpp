#pragma once

#include <Arduino.h>

namespace buzzer {
    constexpr int buzzer_PIN = 39;

    // local setup
    void localSetup() {
        pinMode(buzzer_PIN, OUTPUT);
    }

    // local loop
    void localLoop() {
        for (int i = 0; i < 5; i++) {
            tone(buzzer_PIN, 4000, 500);
            // Serial.println("Alarm triggered with high frequency");
            tone(buzzer_PIN, 2000, 500);
            // Serial.println("Alarm triggered with low frequency");
            noTone(buzzer_PIN);
        }
    }
}