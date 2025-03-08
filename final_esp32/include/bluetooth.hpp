#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <vector>
#include <string>
#include <unordered_map>

#define SCAN_TIME 5  // Scan duration in seconds

namespace bluetooth 
{
    // Bluetooth scanning states 
    enum BluetoothState 
    {
        IDLE,           // Not scanning
        SCANNING,       // Currently scanning
        DEVICE_FOUND,   // Device found during scan
        NO_DEVICE       // No device found after scan
    };
    
    // declare the device map (name -> UUID)
    extern std::unordered_map<std::string, std::string> TARGET_DEVICES;
    
    // Function declarations
    void localSetup();
    void localLoop();
    void setSleepMode(bool sleep);
    bool isDeviceFound();
    
    // External declarations
    extern BLEScan* p_ble_scan;
    extern unsigned long previous_millis;
    extern bool led_state;
    extern BluetoothState current_state;
    extern bool sleep_mode;
    extern std::string last_found_device_name;
} // namespace bluetooth 