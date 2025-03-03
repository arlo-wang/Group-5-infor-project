// Define the camera model before including camera_pins.h
#define CAMERA_MODEL_ESP32S3_EYE  // Has PSRAM; adjust if necessary

//trigger pin for FPGA (set up as an input with pulldown resistor)
#define TRIGGER_PIN 33

#include "camera_pins.h"
#include <Arduino.h>
#include <WiFi.h>

#include "esp_camera.h"
#include "tjpgd.h"  // Tiny JPEG Decoder (if needed)
#include "esp_http_server.h"
#include "esp_heap_caps.h"
#include "base64.h"

// UART settings for FPGA communication
#define UART_TX_PIN 17  // Update according to your wiring
#define UART_RX_PIN 16  // Update according to your wiring

const char *ssid = "zi";
const char *password = "zitongissmart";

// Flag for capture trigger (set by FPGA)
volatile bool captureRequested = false;

// Store last captured barcode image
camera_fb_t *last_captured_fb = NULL;

// ---------------------------
// Interrupt Service Routine for Trigger
//FPGA → ESP32-S3
// ---------------------------
void IRAM_ATTR onTrigger() {
  captureRequested = true;
}

// ---------------------------
// Barcode Detection (Edge Count Method)
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
    return (edgeCount >= min_edges && edgeCount <= max_edges);
}


// ---------------------------
// Capture and Process Image
// ---------------------------
void captureAndProcessImage() {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
        Serial1.println("FPGA: Capture failed");
        return;
    }
  
    Serial.println("📸 Image captured! Checking for barcode...");
    int width = fb->width;
    int height = fb->height;

    // Allocate grayscale image buffer
    uint8_t *grayImg = (uint8_t*)ps_malloc(width * height);
    if (!grayImg) {
        Serial.println("Memory allocation failed");
        Serial1.println("FPGA: Memory alloc failed");
        esp_camera_fb_return(fb);
        return;
    }

    // Convert image to grayscale (simple copy since PIXFORMAT_JPEG is used, you may need proper conversion)
    for (int i = 0; i < width * height; i++) {
        grayImg[i] = fb->buf[i];
    }

    if (detectBarcode(grayImg, width, height)) {
        Serial.println("✅ Barcode detected! Saving image...");
        Serial1.println("FPGA: Barcode detected");

        // Free last stored frame buffer if exists
        if (last_captured_fb) {
            esp_camera_fb_return(last_captured_fb);
        }
        last_captured_fb = fb;
    } else {
        Serial.println("❌ No barcode detected. Image will NOT be uploaded.");
        Serial1.println("FPGA: No barcode detected");
        esp_camera_fb_return(fb);  
    }

    free(grayImg);
}

// Function to simulate cloud data reception
String getCloudData() {
    // Simulate retrieving data from a cloud service (e.g., Firebase, AWS, Google Cloud)
    String cloud_data = "66666";  // Example: Sending number 42 to FPGA
    Serial.printf("📡 Received from cloud: %s\n", cloud_data.c_str());
    return cloud_data;
}

// Function to send data to FPGA via UART
void sendDataToFPGA(String data) {
    Serial.printf("🔄 Sending to FPGA: %s\n", data.c_str());
    Serial1.println(data);  // Send the data over UART to FPGA
}


// ---------------------------
// Setup
// ---------------------------
void setup() {
    Serial.begin(115200);
    Serial.println();
    
    // Initialize trigger pin and attach interrupt (for FPGA trigger signal)
    pinMode(TRIGGER_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), onTrigger, RISING);

    // Initialize UART for FPGA communication on Serial1
    Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
    Serial.println("UART for FPGA initialized.");
    Serial1.println("FPGA: UART initialized");

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
    config.pixel_format = PIXFORMAT_JPEG;  // Capture JPEG
    config.frame_size = FRAMESIZE_QVGA;      // 320x240 resolution
    config.fb_location = CAMERA_FB_IN_PSRAM;  // Use PSRAM if available
    config.jpeg_quality = 10;
    config.fb_count = 2;

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("❌ Camera init failed with error 0x%x\n", err);
        Serial1.println("FPGA: Camera init failed");
        return;
    }
    Serial.println("📷 Camera initialized.");
    Serial1.println("FPGA: Camera init successful");

    // Initialize WiFi (if using cloud-based features)
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);
    Serial.print("WiFi connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial1.println("FPGA: WiFi connected");

    Serial.print("Camera Ready! Connect via http://");
    Serial.println(WiFi.localIP());
    Serial1.print("FPGA: Camera Ready, IP ");
    Serial1.println(WiFi.localIP());
}

// ---------------------------
// Main Loop
// ---------------------------
void loop() {
    if (captureRequested) {
        captureRequested = false;
        Serial.println("⚡ Trigger received -> Capturing image...");
        Serial1.println("FPGA: Trigger received, capturing image...");
        captureAndProcessImage();
    }

    // Fetch data from the cloud every 10 seconds and send it to FPGA
    static unsigned long lastCloudUpdate = 0;
    if (millis() - lastCloudUpdate > 10000) {  // Every 10 seconds
        lastCloudUpdate = millis();
        String cloudData = getCloudData();  // Get data from the cloud
        sendDataToFPGA(cloudData);  // Send it to FPGA over UART
    }

    delay(5000);
}
