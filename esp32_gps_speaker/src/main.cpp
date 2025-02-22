#include <Arduino.h>
#include <WiFi.h>
#include "../include/buzzer.hpp"
#include "../include/wifi.hpp"
// pins used in the project: 2, 40
constexpr int LED_PIN = 2;

void setup() 
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    wifi::localSetup();
    buzzer::localSetup();
}

void loop() 
{   
    // LED keep off if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) digitalWrite(LED_PIN, LOW);
    else digitalWrite(LED_PIN, HIGH);
    
    buzzer::localLoop();
}