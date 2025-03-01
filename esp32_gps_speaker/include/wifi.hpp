#pragma once

#include <Arduino.h>
#include <WiFi.h>

namespace wifi {
    // WiFi settings
    const char* ssid = "VM5892311";
    const char* password = "q6hQzcdmjqwq"; 

    void localSetup()
    {
        // connect to WiFi
        // Serial.printf("Connecting to %s ", ssid);
        WiFi.begin(ssid, password);

        // wait for WiFi connection
        while (WiFi.status() != WL_CONNECTED) {
            // Serial.print("connecting...");
            // Serial.println("WiFi connected");
            // Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
        }
        
        // WiFi connected
        Serial.println("\nWiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    }
} // namespace wifi