#include "../include/gps.hpp"

namespace gps {
    // Initialize global variables
    String nmea_sentence;
    bool new_data = false;
    
    // GPS data initialisation
    GPSData gps = {
        0.0, 0.0, 0.0, 0.0, 0.0,  // Position and movement
        false, 0, 0,              // Status
        0, 0, 0, 0, 0, 0,         // Time
        99.9,                     // HDOP (invalid value)
        0, 0,                     // Last update, last valid fix
        true,                     // Inside geofence (default to true)
        0.0,                      // Distance to center
        0.0                       // Accuracy radius
    };
    
    // Geofence settings initialisation
    Geofence geofence = {
        51.494476, -0.194417,
        100.0,             // Default radius 100 meters
        false,             // Disabled by default
        false              // Alarm not triggered
    };

    // variable to track last update time
    unsigned long last_gps_update = 0;
    // define update interval as 5 seconds
    const unsigned long GPS_UPDATE_INTERVAL = 5000;
    // define GPS signal timeout (10 seconds)
    const unsigned long GPS_SIGNAL_TIMEOUT = 10000;

    // define server URL
    const char* serverUrl = "http://3.8.78.228:8000/api/cart/update/"; 

    // Create JSON string from GPS data
    String createGPSJson(const GPSData &gps) {
        JsonDocument doc;

        doc["id"]  = CART_ID;  
        doc["lat"] = gps.latitude;  
        doc["lng"] = gps.longitude;
        doc["radius"] = gps.accuracy_radius;  

        String jsonString;
        serializeJson(doc, jsonString);
        return jsonString;
    }

    // send GPS data to server 
    void sendGPSDataToServer(const String &jsonData, const String &serverUrl) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");
        
        int httpCode = http.POST(jsonData);
        
        // print HTTP response code
        if (httpCode > 0) DEBUGSerial.printf("HTTP:%d\n", httpCode);
        // print error message
        else DEBUGSerial.printf("ERR:%d\n", httpCode);

        http.end(); // end HTTP connection
    }

    // GPS module initialisation
    void localSetup() 
    {
        // RX : 41 (connect to gps Tx), TX : 42
        // NOTE: 35, 36 cannot be used as UART pins
        GPSSerial.begin(115200, SERIAL_8N1, GPS_RX, GPS_TX);   
        DEBUGSerial.println("GPS Module Test");
        DEBUGSerial.println("Waiting for GPS data...");
        // enable geofence
        setGeofence(geofence.center_latitude, geofence.center_longitude, geofence.radius, true);
    }

    // GPS data reading and parsing
    void localLoop() {
        // a static variable to track the previous location valid status
        static bool previous_location_valid = false;
        
        while (GPSSerial.available()) {
            char c = GPSSerial.read();
            gps.last_update = millis();  // Update last data time
            
            // Collect NMEA sentence
            if (c == '$') nmea_sentence = ""; // start of new NMEA sentence
            
            nmea_sentence += c;
            
            // End of NMEA sentence
            if (c == '\n') {
                // Print the complete NMEA sentence
                // DEBUGSerial.print(nmea_sentence);  
                // Parse the NMEA sentence
                parseNMEA(nmea_sentence, gps);
                // Check geofence status
                checkGeofence(gps, geofence);
                
                // Update last valid fix time if location is valid
                if (gps.location_valid) {
                    gps.last_valid_fix = millis();
                }
                
                new_data = true;
            }
        }
        
        // Check for GPS signal timeout
        if (gps.location_valid && gps.last_valid_fix > 0) {
            unsigned long time_since_last_fix = millis() - gps.last_valid_fix;
            if (time_since_last_fix > GPS_SIGNAL_TIMEOUT) {
                // Mark location as invalid if timeout exceeded
                gps.location_valid = false;
                DEBUGSerial.println("GPS signal lost: No valid data received for over 10 seconds");
            }
        }
        
        // detect GPS signal recovery
        if (gps.location_valid && !previous_location_valid) {
            DEBUGSerial.println("GPS signal recovered - checking geofence status");
            checkGeofence(gps, geofence);
        }
        
        // 更新上一次的位置有效状态
        previous_location_valid = gps.location_valid;
        
        // only update GPS data if the specified time interval has passed
        if (millis() - last_gps_update >= GPS_UPDATE_INTERVAL) {
            // Always display GPS info, regardless of validity
            // This will show "INVALID" status when location is not valid
            displayGPSInfo(gps);
            
            // Check geofence only if location is valid
            if (gps.location_valid) {
                checkGeofence(gps, geofence);
            }
            
            // send GPS data to server
            if (gps.location_valid) {
                // create JSON data
                String jsonData = createGPSJson(gps);
                DEBUGSerial.println("Sending GPS data: " + jsonData);
                
                // send to server
                sendGPSDataToServer(jsonData, serverUrl);
            }
            
            // update last update time
            last_gps_update = millis();
        }
    }

    // Calculate distance between two points (meters) using Haversine formula
    float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
        const float R = 6371000; // Earth radius in meters
        float phi1 = lat1 * M_PI / 180;
        float phi2 = lat2 * M_PI / 180;
        float deltaPhi = (lat2 - lat1) * M_PI / 180;
        float deltaLambda = (lon2 - lon1) * M_PI / 180;

        float a = sin(deltaPhi/2) * sin(deltaPhi/2) +
                cos(phi1) * cos(phi2) *
                sin(deltaLambda/2) * sin(deltaLambda/2);
        float c = 2 * atan2(sqrt(a), sqrt(1-a));
        float distance = R * c;
        
        return distance;
    }

    // Check if within geofence
    void checkGeofence(GPSData &gps, Geofence &fence) {
        if (!fence.enabled || !gps.location_valid) 
        {
            return; // Skip check if geofence is disabled or position is invalid
        }
        
        // Calculate distance to center point
        gps.distance_to_center = calculateDistance(
            gps.latitude, 
            gps.longitude,
            fence.center_latitude, 
            fence.center_longitude
        );
        
        // Determine if inside geofence
        bool was_inside = gps.inside_geofence;
        gps.inside_geofence = (gps.distance_to_center <= fence.radius);
        
        // Trigger alarm if moved from inside to outside
        if (was_inside && !gps.inside_geofence) {
            fence.alarm_triggered = true;
            DEBUGSerial.println("!!! GEOFENCE ALARM: Device has left the geofence area !!!");
        }
        // Reset alarm if moved from outside to inside or if signal just recovered and inside
        else if ((!was_inside && gps.inside_geofence) || gps.inside_geofence) {
            // if device moved from outside to inside, or signal just recovered and inside, reset alarm
            if (fence.alarm_triggered) {
                DEBUGSerial.println("Device is back inside geofence area - resetting alarm");
                fence.alarm_triggered = false;
            }
        }
    }

    // Set geofence parameters
    void setGeofence(float lat, float lon, float radius, bool enabled) {
        geofence.center_latitude = lat;
        geofence.center_longitude = lon;
        geofence.radius = radius;
        geofence.enabled = enabled;
        geofence.alarm_triggered = false; // Reset alarm status
        
        DEBUGSerial.println("Geofence configured:");
        DEBUGSerial.print("Center: "); 
        DEBUGSerial.print(lat, 6); 
        DEBUGSerial.print(", "); 
        DEBUGSerial.println(lon, 6);
        DEBUGSerial.print("Radius: "); 
        DEBUGSerial.print(radius); 
        DEBUGSerial.println(" meters");
        DEBUGSerial.print("Status: "); 
        DEBUGSerial.println(enabled ? "Enabled" : "Disabled");
    }

    // Check if alarm has been triggered
    bool isGeofenceAlarmTriggered() {
        return geofence.alarm_triggered;
    }

    // Reset alarm status
    void resetGeofenceAlarm() {
        geofence.alarm_triggered = false;
    }

    // Check if GPS has valid location
    bool hasValidLocation() {
        return gps.location_valid;
    }
    
    // Get time since last valid fix
    unsigned long getTimeSinceLastValidFix() {
        if (gps.last_valid_fix == 0) {
            return ULONG_MAX; // Never had a valid fix
        }
        return millis() - gps.last_valid_fix;
    }

    // parse NMEA sentence
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
            // If HDOP is available, calculate accuracy radius
            if (hdop_str.length() > 0) {
                gps.hdop = hdop_str.toFloat();
                // Calculate accuracy radius - based on HDOP value
                // Typical GPS receiver basic accuracy is about 2.5 meters
                const float BASE_ACCURACY = 2.5;  // meters
                gps.accuracy_radius = gps.hdop * BASE_ACCURACY;
            }
            
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
                    gps.year = 2000 + date_str.substring(4, 6).toInt();
                }
            }
            else if (status == 'V') gps.location_valid = false;
        }
    }

    // display GPS information
    void displayGPSInfo(const GPSData &gps) {
        DEBUGSerial.println("\n--- GPS Information ---");
        DEBUGSerial.print("Location: ");
        if (gps.location_valid) 
        {
            DEBUGSerial.print(gps.latitude, 6);
            DEBUGSerial.print(", ");
            DEBUGSerial.println(gps.longitude, 6);
            
            // DEBUGSerial.print("Altitude: ");
            // DEBUGSerial.print(gps.altitude);
            // DEBUGSerial.println(" m");
            
            // DEBUGSerial.print("Speed: ");
            // DEBUGSerial.print(gps.speed);
            // DEBUGSerial.println(" knots");
            
            // DEBUGSerial.print("Course: ");
            // DEBUGSerial.print(gps.course);
            // DEBUGSerial.println(" degrees");
            
            // if (gps.year > 0) {
            //     DEBUGSerial.printf("Date/Time: %02d/%02d/%04d %02d:%02d:%02d\n", 
            //         gps.day, gps.month, gps.year, gps.hour, gps.minute, gps.second);
            // }
            
            // Add geofence information
            if (geofence.enabled) {
                // DEBUGSerial.println("\n--- Geofence Details ---");
                // DEBUGSerial.print("Status: ");
                // DEBUGSerial.println(gps.inside_geofence ? "INSIDE ✓" : "OUTSIDE ✗");
                
                // DEBUGSerial.print("Distance to center: ");
                // DEBUGSerial.print(gps.distance_to_center);
                // DEBUGSerial.print(" / ");
                // DEBUGSerial.print(geofence.radius);
                // DEBUGSerial.println(" meters");
                
                if (gps.inside_geofence) {
                    DEBUGSerial.print("Margin: ");
                    DEBUGSerial.print(geofence.radius - gps.distance_to_center);
                    DEBUGSerial.println(" meters from boundary");
                } else {
                    DEBUGSerial.print("Distance beyond boundary: ");
                    DEBUGSerial.print(gps.distance_to_center - geofence.radius);
                    DEBUGSerial.println(" meters");
                }
                
                DEBUGSerial.print("Alarm status: ");
                DEBUGSerial.println(geofence.alarm_triggered ? "TRIGGERED" : "NOT TRIGGERED");
            }
        } 

        // if the location is not valid
        else {
            DEBUGSerial.println("INVALID - NO SIGNAL");
            
            // Show last known position if available
            if (gps.last_valid_fix > 0) {
                DEBUGSerial.println("Last known position (NOT CURRENT):");
                DEBUGSerial.print("  Latitude: ");
                DEBUGSerial.println(gps.latitude, 6);
                DEBUGSerial.print("  Longitude: ");
                DEBUGSerial.println(gps.longitude, 6);
            } else {
                DEBUGSerial.println("No position data available");
            }
            
            DEBUGSerial.print("Time since last valid fix: ");
            if (gps.last_valid_fix > 0) {
                DEBUGSerial.print((millis() - gps.last_valid_fix) / 1000);
                DEBUGSerial.println(" seconds");
            } else {
                DEBUGSerial.println("Never had a valid fix");
            }
        }
        
        // DEBUGSerial.print("Satellites visible: ");
        // DEBUGSerial.println(gps.satellites_visible);
        
        // DEBUGSerial.print("Fix quality: ");
        // DEBUGSerial.println(gps.fix_quality);
        
        // DEBUGSerial.print("HDOP: ");
        // DEBUGSerial.println(gps.hdop);
        
        DEBUGSerial.println("----------------------");
    }

    float getAccuracyRadius() {
        // If location is invalid, return a large value to indicate high uncertainty
        if (!gps.location_valid) {
            return 9999.0;  // indicate high uncertainty when location is invalid
        }
        
        return gps.accuracy_radius;
    }

} // namespace gps 