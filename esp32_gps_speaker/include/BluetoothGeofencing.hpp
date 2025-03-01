// // // // // // // // // // // // // // // // // // //
// // This file has not yet been added to main.cpp // //
// // // // // // // // // // // // // // // // // // //

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define TARGET_UUID "12345678-1234-5678-1234-56789abcdef1"  // Your Service UUID
#define SCAN_TIME 5  // Scan duration in seconds

// LED debug pin definitions
#define LED_PIN 2  // Using ESP32-S3 built-in LED
#define LED_SCAN 500  // LED blink interval during scanning (ms)
#define LED_FOUND 100  // LED fast blink interval when device found (ms)

BLEScan* pBLEScan;
unsigned long previousMillis = 0;
bool ledState = false;

// LED blinking function
void blinkLED(int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-S3 BLE Scanner Initializing...");

    // Initialize LED pin
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // LED on during initialization
    
    BLEDevice::init("ESP32_Scanner");  // Set ESP32 BLE device name
    pBLEScan = BLEDevice::getScan();   // Get scan object
    pBLEScan->setActiveScan(true);     // Enable active scanning (faster discovery)
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);
    
    // Initialization complete, turn off LED
    delay(1000);
    digitalWrite(LED_PIN, LOW);
}

void loop() {
    Serial.println("Scanning for BLE devices...");
    
    // Start scanning, slow LED blinking
    for (int i = 0; i < SCAN_TIME * 1000 / LED_SCAN; i++) {
        blinkLED(LED_SCAN);
        delay(10);  // Small delay to allow LED state update
    }
    
    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
    bool deviceFound = false;

    for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);

        // Check if the advertised service UUID matches TARGET_UUID
        if (device.haveServiceUUID() && device.isAdvertisingService(BLEUUID(TARGET_UUID))) {
            Serial.println("Found target BLE device!");
            Serial.printf("Device Address: %s\n", device.getAddress().toString().c_str());
            Serial.printf("RSSI: %d dBm\n", device.getRSSI());
            deviceFound = true;
            break;  // Stop scanning after first match
        }
    }

    if (deviceFound) {
        // Target device found, LED blinks fast 5 times
        digitalWrite(LED_PIN, LOW);  // Ensure LED starts from off state
        for (int i = 0; i < 10; i++) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            delay(LED_FOUND);
        }
    } else {
        Serial.println("No matching device found.");
        // No device found, LED stays on for 2 seconds
        digitalWrite(LED_PIN, HIGH);
        delay(2000);
        digitalWrite(LED_PIN, LOW);
    }

    delay(1000);  // Wait 1 second before next scan
}