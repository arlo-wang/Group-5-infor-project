// // // // // // // // // // // // // // // // // // //
// // This file has not yet been added to main.cpp // //
// // // // // // // // // // // // // // // // // // //

#include <Arduino.h>
#define GPSSerial Serial2
#define DEBUGSerial Serial

// LED pin definition for GPS data indication
#define LED_PIN 2  // Using ESP32 built-in LED

void parseNMEA(String nmea);
void displayGPSInfo();

// Buffer for storing NMEA sentences
String nmea_sentence;
bool new_data = false;

// GPS data variables
float latitude = 0.0, longitude = 0.0;
bool location_valid = false;
int satellites_visible = 0;
unsigned long last_data_time = 0;

void setup()
{
    GPSSerial.begin(115200, SERIAL_8N1, 16, 17);   // RX : 16, TX : 17
    DEBUGSerial.begin(115200);  
    DEBUGSerial.println("GPS Module Test");
    DEBUGSerial.println("Waiting for GPS data...");
}

void loop()
{
    // LED indication based on GPS status
    // No data for 3 seconds - LED off
    if (millis() - last_data_time > 3000) digitalWrite(LED_PIN, LOW);  

    else if (!location_valid && satellites_visible > 0) 
    {
        // Satellites visible but no valid location - LED blink slowly
        if ((millis() / 1000) % 2 == 0) digitalWrite(LED_PIN, HIGH);
        else digitalWrite(LED_PIN, LOW);
        
    } 

    else if (location_valid) digitalWrite(LED_PIN, HIGH); // Valid location - LED on
    
    // Read GPS data
    while (GPSSerial.available()) {
        char c = GPSSerial.read();
        last_data_time = millis();  // Update last data time
        
        // Collect NMEA sentence
        if (c == '$') nmea_sentence = ""; // start of new NMEA sentence
        
        nmea_sentence += c;
        
        if (c == '\n') 
        {
        // End of NMEA sentence
        DEBUGSerial.print(nmea_sentence);  // Print the complete NMEA sentence
        
        // Parse the NMEA sentence
        parseNMEA(nmea_sentence);
        
        new_data = true;
        }
    }
    
    // If we have new data, display it
    if (new_data) 
    {
        displayGPSInfo();
        new_data = false;
    }
}

void parseNMEA(String nmea) {
    // Check if it's a GGA sentence (contains basic fix data)
    if (nmea.startsWith("$GPGGA") || nmea.startsWith("$GNGGA")) 
    {
        // Split the sentence into parts
        int comma1 = nmea.indexOf(',', 0);
        int comma2 = nmea.indexOf(',', comma1+1);
        int comma3 = nmea.indexOf(',', comma2+1);
        int comma4 = nmea.indexOf(',', comma3+1);
        int comma5 = nmea.indexOf(',', comma4+1);
        int comma6 = nmea.indexOf(',', comma5+1);
        int comma7 = nmea.indexOf(',', comma6+1);
        
        // Extract fix quality (0 = invalid, 1 = GPS fix, 2 = DGPS fix)
        int fix = nmea.substring(comma6+1, comma7).toInt();
        location_valid = (fix > 0);
        
        // Extract number of satellites
        int comma8 = nmea.indexOf(',', comma7+1);
        int sats = nmea.substring(comma7+1, comma8).toInt();
        if (sats > 0) satellites_visible = sats;
        
        // Extract latitude and longitude if fix is valid
        if (location_valid) 
        {
        // Latitude format: ddmm.mmmm
        String lat_str = nmea.substring(comma2+1, comma3);
        float lat_degrees = lat_str.substring(0, 2).toFloat();
        float lat_minutes = lat_str.substring(2).toFloat();
        latitude = lat_degrees + (lat_minutes / 60.0);
        
        // Check N/S indicator
        if (nmea.charAt(comma3+1) == 'S') latitude = -latitude;
        
        // Longitude format: dddmm.mmmm
        String lng_str = nmea.substring(comma4+1, comma5);
        float lng_degrees = lng_str.substring(0, 3).toFloat();
        float lng_minutes = lng_str.substring(3).toFloat();
        longitude = lng_degrees + (lng_minutes / 60.0);
        
        // Check E/W indicator
        if (nmea.charAt(comma5+1) == 'W') longitude = -longitude;
        }
    }
    // Check GSV sentences to count satellites
    else if (nmea.indexOf("GSV") > 0) {
        // GSV sentences contain satellite info
        // We'll just count that we received them to know satellites are visible
        satellites_visible = max(satellites_visible, 1);  // At least one satellite is visible
    }
    
    // Check RMC sentence for validity
    else if (nmea.startsWith("$GPRMC") || nmea.startsWith("$GNRMC")) {
        int comma1 = nmea.indexOf(',', 0);
        int comma2 = nmea.indexOf(',', comma1+1);
        int comma3 = nmea.indexOf(',', comma2+1);
        
        // Check status (A=valid, V=invalid)
        char status = nmea.charAt(comma2+1);
        if (status == 'A') {
        location_valid = true;
        } else if (status == 'V') {
        location_valid = false;
        }
    }
}

void displayGPSInfo() {
    DEBUGSerial.println("\n--- GPS Information ---");
    DEBUGSerial.print("Location: ");
    if (location_valid) {
        DEBUGSerial.print(latitude, 6);
        DEBUGSerial.print(", ");
        DEBUGSerial.println(longitude, 6);
    } else {
        DEBUGSerial.println("INVALID");
    }
    
    DEBUGSerial.print("Satellites visible: ");
    DEBUGSerial.println(satellites_visible);
    DEBUGSerial.println("----------------------");
}
