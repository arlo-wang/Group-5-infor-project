#include "../include/bluetooth.hpp"
#include "../include/gps.hpp"
#include "../include/buzzer.hpp"

namespace bluetooth {
    // Initialize variables
    BLEScan* p_ble_scan = nullptr;
    unsigned long previous_millis = 0;
    bool led_state = false;
    BluetoothState current_state = IDLE;
    bool sleep_mode = false;
    unsigned long last_scan_time = 0;
    std::string last_found_device_name = "";
    
    // define the device map (name -> UUID)
    std::unordered_map<std::string, std::string> TARGET_DEVICES = {
        {"Arlo's iPhone", "12345678-1234-5678-1234-56789abcdef1"},

        // add more devices as needed
    };
    
    // Bluetooth setup
    void localSetup() 
    {
        Serial.println("ESP32-S3 BLE Scanner Initializing...");
        
        BLEDevice::init("ESP32_Scanner");   // Set ESP32 BLE device name
        p_ble_scan = BLEDevice::getScan();  // Get scan object
        p_ble_scan->setActiveScan(true);    // Enable active scanning (faster discovery)
        p_ble_scan->setInterval(1600);      // Set scan interval
        p_ble_scan->setWindow(99);          // Set scan window
        
        Serial.println("Bluetooth scanner initialized");
    }
    
    // Set sleep mode
    void setSleepMode(bool sleep) 
    {
        sleep_mode = sleep;
        if (sleep && current_state == SCANNING) 
        {
            p_ble_scan->stop();
            current_state = IDLE;
            Serial.println("Bluetooth scanner entering sleep mode");
        }
    }

    // Check if a device is found
    bool isDeviceFound() 
    {
        return current_state == DEVICE_FOUND;
    }

    // Bluetooth loop
    void localLoop() 
    {
        // If in sleep mode, do nothing
        if (sleep_mode) 
        {
            return;
        }
        
        // Only scan if we're not already scanning and enough time has passed since last scan
        if (current_state != SCANNING && millis() - last_scan_time > 1000) 
        {
            Serial.println("Scanning for BLE devices...");
            current_state = SCANNING;
            
            // Start scan
            BLEScanResults found_devices = p_ble_scan->start(SCAN_TIME, false);
            last_scan_time = millis();
            
            bool device_found = false;
            
            // Process scan results
            for (int i = 0; i < found_devices.getCount(); i++) 
            {
                BLEAdvertisedDevice device = found_devices.getDevice(i);
                
                // check if the device is advertising any of our target UUIDs
                if (device.haveServiceUUID()) {
                    // iterate through all target devices
                    for (const auto& [device_name, uuid] : TARGET_DEVICES) {
                        if (device.isAdvertisingService(BLEUUID(uuid.c_str()))) {
                            Serial.println("Found target BLE device!");
                            Serial.printf("Device Name: %s\n", device_name.c_str());
                            Serial.printf("Device Address: %s\n", device.getAddress().toString().c_str());
                            Serial.printf("RSSI: %d dBm\n", device.getRSSI());
                            
                            // save the found device name
                            last_found_device_name = device_name;
                            device_found = true;
                            break;  // break the inner loop
                        }
                    }
                    if (device_found) break;  // break the outer loop
                }
            }
            
            // Update state based on scan results
            if (device_found) 
            {
                current_state = DEVICE_FOUND;
                // Stop any Bluetooth-related alarm
                buzzer::stopAlarmIfType(buzzer::BLUETOOTH_ALARM);
            } 
            else 
            {
                current_state = NO_DEVICE;
                last_found_device_name = "";  // reset the device name
                Serial.println("No matching device found.");
                // Trigger alarm if no device found
                buzzer::triggerAlarm(buzzer::BLUETOOTH_ALARM);
            }
            
            // Clear scan results to free memory
            p_ble_scan->clearResults();
        }
    }
} // namespace bluetooth 