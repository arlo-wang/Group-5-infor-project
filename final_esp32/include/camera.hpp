#pragma once

#define CAMERA_MODEL_ESP32S3_EYE
// FPGA trigger pin
#define FPGA_TRIGGER_PIN 21

#include "esp_camera.h"
#include "camera_pins.h"
#include "../src/ws2812.h"
#include "../src/Base64.h" 
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>

namespace camera 
{
    // function declarations
    void localSetup();
    void localLoop();
    int initCamera();
    String urlencode(String str);

    extern const char* server;
}
