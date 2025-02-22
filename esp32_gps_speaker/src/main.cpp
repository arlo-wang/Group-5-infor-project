#include <Arduino.h>
#include <WiFi.h>

constexpr int LED_PIN = 2;

// WiFi settings
const char* ssid = "VM5892311";
const char* password = "q6hQzcdmjqwq"; 


void connectToWiFi()
{
    // connect to WiFi
    Serial.printf("Connecting to %s ", ssid);
    WiFi.begin(ssid, password);

    // wait for WiFi connection
    while (WiFi.status() != WL_CONNECTED) {
        digitalWrite(LED_PIN, HIGH);  // LED flash to show connecting
        delay(1000);
        digitalWrite(LED_PIN, LOW);
        delay(1000);
        Serial.print("connecting...");
    }
    
    // WiFi connected
    digitalWrite(LED_PIN, HIGH);  // LED on to show connected
    Serial.println("\nWiFi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void setup() 
{
    Serial.begin(115200);
    delay(1000);
    pinMode(LED_PIN, OUTPUT);
    connectToWiFi();
}

void loop() 
{
    digitalWrite(LED_PIN, HIGH);  // LED flash to show connecting
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
    if (WiFi.status() == WL_CONNECTED) Serial.println("WiFi connected");
    else Serial.println("WiFi disconnected");
}