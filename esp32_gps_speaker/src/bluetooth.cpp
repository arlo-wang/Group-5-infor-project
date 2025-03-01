#include "../include/bluetooth.hpp"
#include "../include/gps.hpp"
#include "../include/buzzer.hpp"

namespace bluetooth {
    // Initialize variables
    BLEScan* pBLEScan = nullptr;
    unsigned long previousMillis = 0;
    bool ledState = false;
    BluetoothState currentState = IDLE;
    bool sleepMode = false;
    unsigned long lastScanTime = 0;
    
    void localSetup() {
        Serial.println("ESP32-S3 BLE Scanner Initializing...");
        
        BLEDevice::init("ESP32_Scanner");  // Set ESP32 BLE device name
        pBLEScan = BLEDevice::getScan();   // Get scan object
        pBLEScan->setActiveScan(true);     // Enable active scanning (faster discovery)
        pBLEScan->setInterval(100);
        pBLEScan->setWindow(99);
        
        Serial.println("Bluetooth scanner initialized");
    }
    
    void setSleepMode(bool sleep) {
        sleepMode = sleep;
        if (sleep && currentState == SCANNING) {
            pBLEScan->stop();
            currentState = IDLE;
            Serial.println("Bluetooth scanner entering sleep mode");
        }
    }
    
    bool isDeviceFound() {
        return currentState == DEVICE_FOUND;
    }
    
    void localLoop() {
        // If in sleep mode, do nothing
        if (sleepMode) {
            return;
        }
        
        // Only scan if we're not already scanning and enough time has passed since last scan
        if (currentState != SCANNING && millis() - lastScanTime > 1000) {
            Serial.println("Scanning for BLE devices...");
            currentState = SCANNING;
            
            // Start scan
            BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME, false);
            lastScanTime = millis();
            
            bool deviceFound = false;
            
            // Process scan results
            for (int i = 0; i < foundDevices.getCount(); i++) {
                BLEAdvertisedDevice device = foundDevices.getDevice(i);
                
                // Print device info
                Serial.printf("Device found: %s - ", device.getAddress().toString().c_str());
                if (device.haveName()) {
                    Serial.print(device.getName().c_str());
                } else {
                    Serial.print("Unknown");
                }
                Serial.printf(" (RSSI: %d)\n", device.getRSSI());
                
                // Check if the advertised service UUID matches TARGET_UUID
                if (device.haveServiceUUID() && device.isAdvertisingService(BLEUUID(TARGET_UUID))) {
                    Serial.println("Found target BLE device!");
                    Serial.printf("Device Address: %s\n", device.getAddress().toString().c_str());
                    Serial.printf("RSSI: %d dBm\n", device.getRSSI());
                    deviceFound = true;
                    break;  // Stop scanning after first match
                }
            }
            
            // Update state based on scan results
            if (deviceFound) {
                currentState = DEVICE_FOUND;
                // Stop any Bluetooth-related alarm
                buzzer::stopAlarmIfType(buzzer::BLUETOOTH_ALARM);
            } else {
                currentState = NO_DEVICE;
                Serial.println("No matching device found.");
                // Trigger alarm if no device found
                buzzer::triggerAlarm(buzzer::BLUETOOTH_ALARM);
            }
            
            // Clear scan results to free memory
            pBLEScan->clearResults();
        }
    }
} // namespace bluetooth 