#include <Arduino.h>
#include "esp_camera.h"
#include "tjpgd.h"  // Tiny JPEG Decoder
#include "esp_http_server.h"
#include "esp_heap_caps.h"
#include <WiFi.h>

#define CAMERA_MODEL_ESP32S3_EYE  // Has PSRAM
#include "camera_pins.h"
// Define Trigger Pin from FPGA
#define TRIGGER_PIN 2  

const char *ssid = "zi";
const char *password = "zitongissmart";

// Flag for capture trigger (set by FPGA)
volatile bool captureRequested = false;

// Store last captured barcode image
camera_fb_t *last_captured_fb = NULL;

// ---------------------------
// Detect Barcode Presence (Not Decoding, Only Detection)
// ---------------------------
bool detectBarcode(uint8_t* grayImg, int width, int height) {
    int threshold = 25;  // Lowered threshold to detect more edges
    int min_edges = 20;  // Reduced minimum required edges
    int max_edges = 250; // Upper limit to avoid false positives
    int edgeCount = 0;

    int startX = width / 4;    // Scan middle region
    int endX = 3 * width / 4;
    int centerY = height / 2;  // Middle horizontal line

    // Scan horizontally through the center row
    for (int x = startX; x < endX; x++) {
        int diff = abs(grayImg[centerY * width + x] - grayImg[centerY * width + x - 1]);
        if (diff > threshold) {
            edgeCount++;
        }
    }

    Serial.printf("Edge count: %d\n", edgeCount);

    // More lenient barcode detection condition
    return (edgeCount >= min_edges && edgeCount <= max_edges);
}

// ---------------------------
// ESP32 Trigger Interrupt
// ---------------------------
void IRAM_ATTR onTrigger() {
    captureRequested = true;
}

void startCameraServer();

// ---------------------------
// Capture and Process Image
// ---------------------------
void captureAndProcessImage() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("❌ Camera capture failed");
        return;
    }
  
    Serial.println("📸 Image captured! Checking for barcode...");
    int width = fb->width;
    int height = fb->height;

    // Perform grayscale conversion (for barcode detection)
    uint8_t *grayImg = (uint8_t*)ps_malloc(width * height);
    if (!grayImg) {
        Serial.println("❌ Memory allocation failed");
        esp_camera_fb_return(fb);
        return;
    }

    for (int i = 0; i < width * height; i++) {
        grayImg[i] = fb->buf[i];  // Convert to grayscale
    }

    if (detectBarcode(grayImg, width, height)) {
        Serial.println("✅ Barcode detected! Saving image...");
        
        // Free last stored frame buffer
        if (last_captured_fb) {
            esp_camera_fb_return(last_captured_fb);
        }

        // Store the new captured frame buffer
        last_captured_fb = fb;
    } else {
        Serial.println("❌ No barcode detected. Image will NOT be uploaded.");
        
        // If no barcode is detected, **DO NOT update last_captured_fb**
        esp_camera_fb_return(fb);  
    }

    free(grayImg);
}


// ---------------------------
// Setup
// ---------------------------
void setup() {
    Serial.begin(115200);
    Serial.println();

    pinMode(TRIGGER_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), onTrigger, RISING);
    
    // Camera Initialization
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_QVGA;
    config.pixel_format = PIXFORMAT_JPEG;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("❌ Camera init failed with error 0x%x\n", err);
        return;
    }
    
    Serial.println("📷 Camera initialized.");

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("WiFi connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

}

// ---------------------------
// Main Loop
// ---------------------------
void loop() {
    if (captureRequested) {
        captureRequested = false;
        Serial.println("⚡ Trigger received -> Capturing image...");
        captureAndProcessImage();
    }
    delay(100);
}
