#include <Arduino.h>
#include <WiFi.h>

#include "../include/buzzer.hpp"
#include "../include/wifi.hpp"
#include "../include/gps.hpp"
#include "../include/bluetooth.hpp"
#include "../include/LCD.hpp"
// pins used in the project: 39, 16, 17
constexpr int LED_PIN = 2;

// LCD pins 
constexpr int TFT_CS = 15;
constexpr int TFT_DC = 5;
constexpr int TFT_RST = 4;

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// bluetooth task
void bluetoothTask(void *pvParameters) {
    while (true) {
        // Only run Bluetooth scanning if GPS doesn't have a valid location
        if (!gps::hasValidLocation()) {
            bluetooth::setSleepMode(false);  // Ensure Bluetooth is active
            bluetooth::localLoop();
        } else {
            // If GPS has valid location, put Bluetooth in sleep mode
            bluetooth::setSleepMode(true);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // delay to avoid task blocking
    }
}

// gps task
void gpsTask(void *pvParameters) {
    while (true) {
        gps::localLoop();
        vTaskDelay(10 / portTICK_PERIOD_MS); // delay to avoid task blocking
    }
}

// buzzer task
void buzzerTask(void *pvParameters) {
    while (true) {
        buzzer::localLoop();
        vTaskDelay(10 / portTICK_PERIOD_MS); // delay to avoid task blocking
    }
}

void LCDTask(void *pvParameters) {
    while (true) {
        LCD::localLoop();
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// other task
void otherTask(void *pvParameters) {
    while (true) {
        // LED keep on if buzzer is on
        if (buzzer::current_state == buzzer::ALARM) digitalWrite(LED_PIN, HIGH);
        else digitalWrite(LED_PIN, LOW);

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
    bluetooth::localSetup();
    gps::localSetup();
    LCD::localSetup();

    // buzzer task
    xTaskCreatePinnedToCore(
        buzzerTask,    // task function
        "Buzzer Task", // task name
        10000,         // stack size
        NULL,          // task parameters
        1,             // priority
        NULL,          // task handle
        0              // run on core 0
    );

    // bluetooth task
    xTaskCreatePinnedToCore(
        bluetoothTask,    // task function
        "Bluetooth Task", // task name
        10000,         // stack size
        NULL,          // task parameters
        1,             // priority  
        NULL,          // task handle
        0              // run on core 0
    );

    // gps task
    xTaskCreatePinnedToCore(
        gpsTask,    // task function
        "GPS Task", // task name
        10000,      // stack size
        NULL,       // task parameters
        1,          // priority
        NULL,       // task handle
        1           // run on core 1
    );

    xTaskCreatePinnedtoCore(
        LCDTask,    // task function
        "LCD Task", // task name
        10000,        // stack size
        NULL,         // task parameters
        1,            // priority
        NULL,         // task handle
        1             // run on core 1
    );

    // other task
    xTaskCreatePinnedToCore(
        otherTask,    // task function
        "Other Task", // task name
        10000,        // stack size
        NULL,         // task parameters
        1,            // priority
        NULL,         // task handle
        1             // run on core 1
    );

    
}

// loop function
void loop() 
{   
    // do nothing since tasks are running
}