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
    std::vector<std::string> last_found_device_names;
    // std::string last_found_device_name = "";
    
    // define the device map (name -> UUID)
    std::unordered_map<std::string, std::string> TARGET_DEVICES = {
        {"Arlo's iPhone", "12345678-1234-5678-1234-56789abcdef1"},
        {"Zitong's iPhone", "12345678-1234-5678-1234-56789abcdef2"},
        {"Colby's iPhone", "12345678-1234-5678-1234-56789abcdef3"},
        {"Enxing's iPhone", "12345678-1234-5678-1234-56789abcdef4"},
        {"Cole's iPhone", "12345678-1234-5678-1234-56789abcdef5"}
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
    bool isDeviceFound() {
        return current_state == DEVICE_FOUND;
    }

    // Bluetooth loop
    void localLoop() 
    {
        // If in sleep mode, do nothing
        if (sleep_mode) return;
        
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
            // Clear the vector before adding new devices
            last_found_device_names.clear();
            
            for (int i = 0; i < found_devices.getCount(); i++) 
            {
                BLEAdvertisedDevice device = found_devices.getDevice(i);
                
                // check if the device is advertising any of our target UUIDs
                if (device.haveServiceUUID()) {
                    // iterate through all target devices
                    for (const auto& [device_name, uuid] : TARGET_DEVICES) {
                        if (device.isAdvertisingService(BLEUUID(uuid.c_str()))) {
                            // Serial.println("--------------------------------");
                            // Serial.println("Found target BLE device!");
                            // Serial.printf("Device Name: %s\n", device_name.c_str());
                            // Serial.printf("Device Address: %s\n", device.getAddress().toString().c_str());
                            // Serial.printf("RSSI: %d dBm\n", device.getRSSI());
                            // Serial.println("--------------------------------");
                            // save the found device name
                            last_found_device_names.push_back(device_name);
                            device_found = true;
                            // Don't break, continue to find all matching devices
                        }
                    }
                }
            }
            
            // Print all found devices
            if (!last_found_device_names.empty()) {
                Serial.println("All found devices:");
                for (const auto& name : last_found_device_names) {
                    Serial.printf("- %s\n", name.c_str());
                }
                Serial.println("--------------------------------");
            }
            
            // Update state based on scan results
            if (device_found) 
            {
                current_state = DEVICE_FOUND;
                // Stop any Bluetooth-related alarm
                buzzer::stopAlarmIfType(buzzer::ALARM);
            }
             
            // if no device found, trigger alarm
            else 
            {
                current_state = NO_DEVICE;
                last_found_device_names = {};  // reset the device name
                Serial.println("No matching device found.");
                buzzer::triggerAlarm(buzzer::ALARM);
            }
            
            // Clear scan results to free memory
            p_ble_scan->clearResults();
        }
    }
} // namespace bluetooth 