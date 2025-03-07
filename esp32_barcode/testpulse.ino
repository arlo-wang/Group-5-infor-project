// Define the FPGA trigger pin (adjust if needed)
#define FPGA_TRIGGER_PIN 12

// Global flag set by the interrupt when a trigger is received
volatile bool triggerReceived = false;

// Interrupt Service Routine (ISR) for the FPGA trigger
void IRAM_ATTR onFPGATrigger() {
  triggerReceived = true;
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("Waiting for FPGA trigger...");

  // Set the trigger pin as input with internal pull-down resistor
  pinMode(FPGA_TRIGGER_PIN, INPUT_PULLDOWN);
  
  // Attach an interrupt on the trigger pin for a rising edge
  attachInterrupt(digitalPinToInterrupt(FPGA_TRIGGER_PIN), onFPGATrigger, RISING);
}

void loop() {
  // Check if the trigger has been received
  if (triggerReceived) {
    Serial.println("FPGA trigger received!");
    // Clear the flag
    triggerReceived = false;
  }
  // Add a small delay to avoid flooding the serial monitor
  delay(100);
}
