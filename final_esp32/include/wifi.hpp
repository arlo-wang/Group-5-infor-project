#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace wifi {
    // WiFi settings
    const char* ssid = "VM5892311";
    const char* password = "q6hQzcdmjqwq"; 
    // const char* ssid = "Arlo";
    // const char* password = "arloissmart"; 

    void localSetup()
    {
        // connect to WiFi
        // Serial.printf("Connecting to %s ", ssid);
        WiFi.begin(ssid, password);

        // wait for WiFi connection
        while (WiFi.status() != WL_CONNECTED) {
            Serial.print("connecting...");
            delay(500);
        }
        
        // Print WiFi info for threee times when WiFi connected
        for(int i = 0; i < 3; i++) {
            Serial.println("\nWiFi connected!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            delay(100);
        }
    }
} // namespace wifi