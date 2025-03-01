#pragma once

#include <Arduino.h>

#define GPSSerial Serial2
#define DEBUGSerial Serial

// #define LED_PIN 2  

namespace gps {
    // GPS data structure
    struct GPSData {
    // Position data
    float latitude;
    float longitude;
    float altitude;
    float speed;       // Speed in knots
    float course;      // Course in degrees
    
    // Status information
    bool location_valid;
    int satellites_visible;
    int fix_quality;   // 0=invalid, 1=GPS fix, 2=DGPS fix
    
    // Time information
    int hour;
    int minute;
    int second;
    int day;
    int month;
    int year;
    
    // Signal quality
    float hdop;        // Horizontal dilution of precision
    
    // Tracking
    unsigned long last_update;  // Timestamp of last data update
    };

    // Function declarations
    void parseNMEA(String nmea, GPSData &gps);
    void displayGPSInfo(const GPSData &gps);
    // void updateLED(const GPSData &gps);

    // Buffer for storing NMEA sentences
    String nmea_sentence;
    bool new_data = false;

    // GPS data instance
    GPSData gps = {
    0.0, 0.0, 0.0, 0.0, 0.0,  // Position and movement
    false, 0, 0,              // Status
    0, 0, 0, 0, 0, 0,         // Time
    99.9,                     // HDOP (invalid value)
    0                         // Last update
    };

    void localSetup()
    {
        GPSSerial.begin(115200, SERIAL_8N1, 16, 17);   // RX : 16, TX : 17
        DEBUGSerial.begin(115200);  
        DEBUGSerial.println("GPS Module Test");
        DEBUGSerial.println("Waiting for GPS data...");
        
        // Initialize LED pin
        // pinMode(LED_PIN, OUTPUT);
        // digitalWrite(LED_PIN, LOW);
    }

    void localLoop()
    {
        // Update LED based on GPS status
        // updateLED(gps);
        
        // Read GPS data
        while (GPSSerial.available()) 
        {
            char c = GPSSerial.read();
            gps.last_update = millis();  // Update last data time
            
            // Collect NMEA sentence
            if (c == '$') nmea_sentence = ""; // start of new NMEA sentence
            
            nmea_sentence += c;
            
            // End of NMEA sentence
            if (c == '\n') 
            {
                // Print the complete NMEA sentence
                DEBUGSerial.print(nmea_sentence);  
                
                // Parse the NMEA sentence
                parseNMEA(nmea_sentence, gps);
                
                new_data = true;
            }
        }
        
        // If we have new data, display it
        if (new_data) 
        {
            displayGPSInfo(gps);
            new_data = false;
        }
    }

    // // Update LED based on GPS status
    // void updateLED(const GPSData &gps)
    // {
    //     // No data for 3 seconds - LED off
    //     if (millis() - gps.last_update > 3000) digitalWrite(LED_PIN, LOW);

    //     // Satellites visible but no valid location - LED blink slowly
    //     else if (!gps.location_valid && gps.satellites_visible > 0) 
    //     {
    //         if ((millis() / 1000) % 2 == 0) digitalWrite(LED_PIN, HIGH);
    //         else digitalWrite(LED_PIN, LOW);
    //     }
    //     // Valid location - LED on
    //     else if (gps.location_valid) digitalWrite(LED_PIN, HIGH);
    // }

    void parseNMEA(String nmea, GPSData &gps) {
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
            gps.fix_quality = nmea.substring(comma6+1, comma7).toInt();
            gps.location_valid = (gps.fix_quality > 0);
            
            // Extract number of satellites
            int comma8 = nmea.indexOf(',', comma7+1);
            int sats = nmea.substring(comma7+1, comma8).toInt();
            if (sats > 0) gps.satellites_visible = sats;
            
            // Extract time
            String time_str = nmea.substring(comma1+1, comma2);
            if (time_str.length() >= 6) 
            {
                gps.hour = time_str.substring(0, 2).toInt();
                gps.minute = time_str.substring(2, 4).toInt();
                gps.second = time_str.substring(4, 6).toInt();
            }
            
            // Extract HDOP
            int comma9 = nmea.indexOf(',', comma8+1);
            String hdop_str = nmea.substring(comma8+1, comma9);
            if (hdop_str.length() > 0) gps.hdop = hdop_str.toFloat();
            
            // Extract altitude
            int comma10 = nmea.indexOf(',', comma9+1);
            String alt_str = nmea.substring(comma9+1, comma10);
            if (alt_str.length() > 0) gps.altitude = alt_str.toFloat();
            
            // Extract latitude and longitude if fix is valid
            if (gps.location_valid) 
            {
                // Latitude format: ddmm.mmmm
                String lat_str = nmea.substring(comma2+1, comma3);
                float lat_degrees = lat_str.substring(0, 2).toFloat();
                float lat_minutes = lat_str.substring(2).toFloat();
                gps.latitude = lat_degrees + (lat_minutes / 60.0);
                
                // Check N/S indicator
                if (nmea.charAt(comma3+1) == 'S') gps.latitude = -gps.latitude;
                
                // Longitude format: dddmm.mmmm
                String lng_str = nmea.substring(comma4+1, comma5);
                float lng_degrees = lng_str.substring(0, 3).toFloat();
                float lng_minutes = lng_str.substring(3).toFloat();
                gps.longitude = lng_degrees + (lng_minutes / 60.0);
                
                // Check E/W indicator
                if (nmea.charAt(comma5+1) == 'W') gps.longitude = -gps.longitude;
            }
        }
        // Check GSV sentences to count satellites
        else if (nmea.indexOf("GSV") > 0) 
        {
            // GSV sentences contain satellite info
            // We'll just count that we received them to know satellites are visible
            gps.satellites_visible = max(gps.satellites_visible, 1);  // At least one satellite is visible
        }
        
        // Check RMC sentence for validity and additional data
        else if (nmea.startsWith("$GPRMC") || nmea.startsWith("$GNRMC")) 
        {
            int comma1 = nmea.indexOf(',', 0);
            int comma2 = nmea.indexOf(',', comma1+1);
            int comma3 = nmea.indexOf(',', comma2+1);
            
            // Check status (A=valid, V=invalid)
            char status = nmea.charAt(comma2+1);
            if (status == 'A') 
            {
                gps.location_valid = true;
                
                // Extract speed and course
                int comma7 = nmea.indexOf(',', comma3);
                for (int i = 0; i < 3; i++) comma7 = nmea.indexOf(',', comma7+1);
                int comma8 = nmea.indexOf(',', comma7+1);
                int comma9 = nmea.indexOf(',', comma8+1);
                
                // Speed in knots
                String speed_str = nmea.substring(comma7+1, comma8);
                if (speed_str.length() > 0) gps.speed = speed_str.toFloat();
                
                // Course in degrees
                String course_str = nmea.substring(comma8+1, comma9);
                if (course_str.length() > 0) gps.course = course_str.toFloat();
                
                // Extract date
                int comma10 = nmea.indexOf(',', comma9+1);
                String date_str = nmea.substring(comma9+1, comma10);
                if (date_str.length() >= 6) 
                {
                    gps.day = date_str.substring(0, 2).toInt();
                    gps.month = date_str.substring(2, 4).toInt();
                    gps.year = 2000 + date_str.substring(4, 6).toInt(); // Assuming 21st century
                }
            }
            else if (status == 'V') gps.location_valid = false;
        }
    }

    void displayGPSInfo(const GPSData &gps) {
        DEBUGSerial.println("\n--- GPS Information ---");
        DEBUGSerial.print("Location: ");
        if (gps.location_valid) 
        {
            DEBUGSerial.print(gps.latitude, 6);
            DEBUGSerial.print(", ");
            DEBUGSerial.println(gps.longitude, 6);
            
            DEBUGSerial.print("Altitude: ");
            DEBUGSerial.print(gps.altitude);
            DEBUGSerial.println(" m");
            
            DEBUGSerial.print("Speed: ");
            DEBUGSerial.print(gps.speed);
            DEBUGSerial.println(" knots");
            
            DEBUGSerial.print("Course: ");
            DEBUGSerial.print(gps.course);
            DEBUGSerial.println(" degrees");
            
            if (gps.year > 0) {
                DEBUGSerial.printf("Date/Time: %02d/%02d/%04d %02d:%02d:%02d\n", 
                    gps.day, gps.month, gps.year, gps.hour, gps.minute, gps.second);
            }
        } 
        else DEBUGSerial.println("INVALID");
        
        DEBUGSerial.print("Satellites visible: ");
        DEBUGSerial.println(gps.satellites_visible);
        
        DEBUGSerial.print("Fix quality: ");
        DEBUGSerial.println(gps.fix_quality);
        
        DEBUGSerial.print("HDOP: ");
        DEBUGSerial.println(gps.hdop);
        
        DEBUGSerial.println("----------------------");
    }
} // namespace gps