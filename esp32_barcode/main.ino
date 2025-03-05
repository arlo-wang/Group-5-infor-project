// Define the camera model before including camera_pins.h
#define CAMERA_MODEL_ESP32S3_EYE  // Has PSRAM

// Define the trigger pin for FPGA (digital input with pulldown)
// #define TRIGGER_PIN 33

#include "camera_pins.h"
#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
//#include "ws2812.h" //led status indication
#include "tjpgd.h"        // Tiny JPEG Decoder 
#include "esp_http_server.h"
#include "esp_heap_caps.h"
#include "Base64.h"
#include <HTTPClient.h>  // Include HTTP client library

// // UART settings for FPGA communication
// #define UART_TX_PIN 17    // Adjust to your wiring
// #define UART_RX_PIN 18    // Adjust to your wiring

// WiFi credentials
const char *ssid = "zi";
const char *password = "zitongissmart";

// Server endpoint for image upload
const char *server_url = "http://3.8.78.228:8000/api/products/";

const char b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// // Global flags and variables
// volatile bool captureRequested = false;   // Set by FPGA trigger (via interrupt)
// bool scanningAllowed = false;               // Set after FPGA responds to request
//camera_fb_t *last_captured_fb = NULL;       // Holds the last captured image

// ---------------------------
// Interrupt Service Routine for Trigger
// ---------------------------
// void IRAM_ATTR onTrigger() {
//   if (scanningAllowed) {
//     captureRequested = true;
//   }
// }

// ---------------------------
// Wait for FPGA response over UART
// // ---------------------------
// bool waitForFPGAResponse(unsigned long timeout_ms) {
//   unsigned long start = millis();
//   String received = "";
  
//   while (millis() - start < timeout_ms) {
//     while (Serial1.available() > 0) {
//       char ch = Serial1.read();
//       Serial.print(ch);  // Print every received character for debugging
      
//       if (ch == '\n' || ch == '\r') {
//         if (received.length() > 0) {
//           Serial.printf("\n✅ Received from FPGA: %s\n", received.c_str());
          
//           if (received == "on") {
//             return true;  // FPGA allows scanning
//           } else if (received == "off") {
//             return false; // FPGA denies scanning
//           }

//           received = "";  // Reset buffer
//         }
//       } else {
//         received += ch;
//       }
//     }
//     delay(10);
//   }
  
//   return false;  // Timeout
// }

// ---------------------------
// Send data to FPGA via UART (send ASCII string)
// ---------------------------
// void sendDataToFPGA(const String &data) {
//   Serial.printf("Sending to FPGA: %s\n", data.c_str());
//   Serial1.println(data);
// }

// Connect to WiFi
void connectWiFi() {
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  Serial.print("📡 Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected!");
  Serial.print("📶 IP Address: ");
  Serial.println(WiFi.localIP());
}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

// ---------------------------
// Capture and Process Image
// ---------------------------
// Capture image, check for barcode, and upload if detected
String captureAndProcessImage() {
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return "";
  }
  
  Serial.println("Image captured! Checking for barcode...");
  int width = fb->width;
  int height = fb->height;

  // Allocate a buffer for a grayscale image
  uint8_t *grayImg = (uint8_t*)ps_malloc(width * height);
  if (!grayImg) {
    Serial.println("Memory allocation failed");
    esp_camera_fb_return(fb);
    return "";
  }

  // NOTE: This is a simplistic conversion.
  // For a true grayscale conversion from a JPEG image, decoding is required.
  for (int i = 0; i < width * height; i++) {
    grayImg[i] = fb->buf[i];
  }
  String imageFile = "";
  // Run barcode detection on the grayscale image
  Serial.println("Barcode detected! Uploading image...");
  char *input = (char *)fb->buf;
  char output[base64_enc_len(3)];
    
  for (int i=0;i<fb->len;i++) {
    base64_encode(output, (input++), 3);
    if (i%3==0) imageFile += urlencode(String(output));
  }
  Serial.println(imageFile);
  
  
  free(grayImg);
  esp_camera_fb_return(fb);

  Serial.println("📡 Uploading image to server...");

  // Initialize HTTP client
  HTTPClient http;
  http.begin(server_url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = http.POST("image="+imageFile);
  String response = "";

  if (httpResponseCode > 0) {
      Serial.printf("✅ Server response: %d\n", httpResponseCode);
      response = http.getString();
      Serial.printf("📡 Server replied: %s\n", response.c_str());
  } else {
      Serial.printf("❌ HTTP POST failed! Error code: %d\n", httpResponseCode);
      response = "ERROR";
  }

    // Close connection
  http.end();
  return response;
}




// String getCloudData() {
//     return "888";  // Example: Simulate getting data from the cloud
// }


// ---------------------------
// Barcode Detection Function (Edge Count Method)
// ---------------------------
// bool detectBarcode(uint8_t* grayImg, int width, int height) {
//   int threshold = 25;  // Threshold for edge detection
//   int min_edges = 20;  // Minimum number of edges required
//   int max_edges = 250; // Maximum allowed edges (to avoid false positives)
//   int edgeCount = 0;

//   int startX = width / 4;    // Middle region start
//   int endX = 3 * width / 4;    // Middle region end
//   int centerY = height / 2;    // Middle row

//   for (int x = startX; x < endX; x++) {
//     int diff = abs(grayImg[centerY * width + x] - grayImg[centerY * width + x - 1]);
//     if (diff > threshold) {
//       edgeCount++;
//     }
//   }
  
//   Serial.printf("Edge count: %d\n", edgeCount);
//   return (edgeCount >= min_edges && edgeCount <= max_edges);
// }

// ---------------------------
// Setup camera
// ---------------------------
int cameraSetup(void) {
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 4;
  config.fb_count = 1;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  // for larger pre-allocated frame buffer.
  if(psramFound()){
    config.jpeg_quality = 4;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    // Limit the frame size when PSRAM is not available
    config.frame_size = FRAMESIZE_240X240;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return 0;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  s->set_vflip(s, 1); // flip it back
  s->set_brightness(s, 1); // up the brightness just a bit
  s->set_saturation(s, 0); // lower the saturation

  Serial.println("Camera configuration complete!");
  return 1;
}
// ---------------------------
// Setup Function
// ---------------------------

void setup() {
  Serial.begin(115200);
  Serial.println("\n🚀 ESP32 Barcode Scanner Starting...");
  
  // Initialize ws2812 LED (if used)
  //ws2812Init();
  
  connectWiFi();
  cameraSetup();

  // Use internal pull-up for button trigger
 pinMode(BOOT_PIN, INPUT_PULLUP);
  
  // Indicate ready status (color 2, adjust per your ws2812 implementation)
//  ws2812SetColor(2);
}
  
  // // Initialize trigger pin and attach interrupt (for FPGA trigger signal)
  // pinMode(TRIGGER_PIN, INPUT_PULLDOWN);
  // attachInterrupt(digitalPinToInterrupt(TRIGGER_PIN), onTrigger, RISING);
  
  // Initialize UART for FPGA communication on Serial1
  // Serial1.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  // Serial.println("UART for FPGA initialized.");
  // Serial1.println("FPGA: UART initialized");
  
  // ---------- Handshake: Request FPGA Permission to Start Scanning ----------
  // Send a request to the FPGA asking if scanning can start
  // sendDataToFPGA("SCAN_REQUEST");
  
  // Wait up to 5000ms for FPGA response "on"
  // if (waitForFPGAResponse(5000)) {
  //   scanningAllowed = true;
  //   Serial.println("FPGA responded: on. Scanning is allowed.");
  //   //sendDataToFPGA("ESP32: Ready to scan");
  // } else {
  //   scanningAllowed = false;
  //   Serial.println("No response from FPGA. Scanning disabled.");
  //   sendDataToFPGA("ESP32: Scanning disabled");

  

// ---------------------------
// Main Loop
// ---------------------------
void loop() {
  // Check for button press (active LOW)
  if(digitalRead(BOOT_PIN) == LOW) {
    delay(20); // Simple debounce
    if(digitalRead(BOOT_PIN) == LOW) {
      //ws2812SetColor(3);  // Indicate capture in progress
      while(digitalRead(BOOT_PIN) == LOW);  // Wait until button is released

      captureAndProcessImage();
      
      // Return LED to "ready" status
      //ws2812SetColor(2);
    }
  }
  delay(100);  // Small delay to reduce busy-looping


  // Only scan if FPGA has allowed scanning via handshake and trigger is received
  // if (scanningAllowed && captureRequested) {
  //   captureRequested = false;
  //   Serial.println("Trigger received -> Capturing image...");
  //   Serial1.println("FPGA: Trigger received, capturing image...");
  //   captureAndProcessImage();
  // }
  
  // // Periodically, every 10 seconds, fetch data from the cloud and send it to FPGA
  // static unsigned long lastCloudUpdate = 0;
  // if (millis() - lastCloudUpdate > 1000) {
  //   lastCloudUpdate = millis();
  //   String cloudData = getCloudData();  // Retrieve data (e.g., price) from cloud
  //   sendDataToFPGA(cloudData);           // Send it to FPGA via UART
  // }