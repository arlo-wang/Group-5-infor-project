#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <map>

// Pin definitions for TFT display
#define TFT_CS     15    // Chip Select Pin
#define TFT_RST    4     // Reset Pin
#define TFT_DC     2     // Data/Command Pin


void setupDisplayServer();

void loopDisplayServer();


String decode_msg(String Keywd, String Payload);


bool setColor(const String& colorName, uint16_t& colorValue);

// External declarations for global variables
extern Adafruit_ST7735 tft;
extern WiFiServer server;
extern const std::map<String, uint16_t> colorMap;

