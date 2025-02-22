#include <Arduino.h>

#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  Serial.println("Blinking LED...");
  
  digitalWrite(LED_PIN, HIGH); // LED ON
  delay(500);
  
  digitalWrite(LED_PIN, LOW);  // LED OFF
  delay(500);
}