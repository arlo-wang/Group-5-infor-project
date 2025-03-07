#pragma once

#include <Arduino.h>
#include <WiFi.h>

#include <Adafruit_ST7735.h> // Graphics library and LCD control functions
#include <Adafruit_GFX.h>


namespace LCD {
    // WiFi settings
    // const char* ssid = "VM5892311";
    // const char* password = "q6hQzcdmjqwq"; 
    const int port = 12000;
    WiFiServer server(serverPort);

    void localSetup(){}

    void localLoop(){}

    String decode_msg(String Keywd, String Payload){}

} // namespace wifi