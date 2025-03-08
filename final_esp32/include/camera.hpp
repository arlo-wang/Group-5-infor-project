#pragma once

#define CAMERA_MODEL_ESP32S3_EYE
#define BUTTON_PIN  0

#include "esp_camera.h"
#include "camera_pins.h"
#include "../lib/ws2812.h"
#include "../lib/Base64.h" 
#include <WiFi.h>
#include <HTTPClient.h>

namespace camera 
{
    // function declarations
    void localSetup();
    void localLoop();
    int initCamera();
    String urlencode(String str);

    extern const char* server;
}
