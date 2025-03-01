#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define TARGET_UUID "12345678-1234-5678-1234-56789abcdef1"  // Your Service UUID
#define SCAN_TIME 5  // Scan duration in seconds

namespace bluetooth {
    // Bluetooth scanning states
    enum BluetoothState {
        IDLE,           // Not scanning
        SCANNING,       // Currently scanning
        DEVICE_FOUND,   // Device found during scan
        NO_DEVICE       // No device found after scan
    };
    
    // Function declarations
    void localSetup();
    void localLoop();
    void setSleepMode(bool sleep);
    bool isDeviceFound();
    
    // External declarations
    extern BLEScan* pBLEScan;
    extern unsigned long previousMillis;
    extern bool ledState;
    extern BluetoothState currentState;
    extern bool sleepMode;
} // namespace bluetooth 