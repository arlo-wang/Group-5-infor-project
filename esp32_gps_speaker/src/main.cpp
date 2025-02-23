#include <Arduino.h>
#include <WiFi.h>
#include "../include/buzzer.hpp"
#include "../include/wifi.hpp"
// pins used in the project: 2, 39
constexpr int LED_PIN = 2;

// buzzer task
void buzzerTask(void *pvParameters) {
    while (true) {
        buzzer::localLoop();
        vTaskDelay(10 / portTICK_PERIOD_MS); // delay to avoid task blocking
    }
}

// other task
void otherTask(void *pvParameters) {
    while (true) {
        // LED keep off if WiFi is connected
        if (WiFi.status() == WL_CONNECTED) digitalWrite(LED_PIN, LOW);
        else digitalWrite(LED_PIN, HIGH);

        vTaskDelay(1000 / portTICK_PERIOD_MS); // delay to avoid task blocking
    }
}

// setup function
void setup() 
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    wifi::localSetup();
    buzzer::localSetup();

    xTaskCreatePinnedToCore(
        buzzerTask,    // task function
        "Buzzer Task", // task name
        10000,         // stack size
        NULL,          // task parameters
        1,             // priority
        NULL,          // task handle
        1              // run on core 1
    );

    xTaskCreatePinnedToCore(
        otherTask,    // task function
        "Other Task", // task name
        10000,        // stack size
        NULL,         // task parameters
        1,            // priority
        NULL,         // task handle
        0             // run on core 0
    );
}

// loop function
void loop() 
{   
    // do nothing since tasks are running
}