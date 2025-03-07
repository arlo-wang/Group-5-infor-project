#pragma once

#include <Arduino.h>
#include <math.h>  // For distance calculation
#include <HTTPClient.h>  // add HTTP client library
#include <ArduinoJson.h>  // add JSON library

// Serial port definitions
#define GPSSerial Serial2
#define DEBUGSerial Serial
#define CART_ID "1"

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
        unsigned long last_valid_fix;  // Timestamp of last valid fix
        
        // Geofence status
        bool inside_geofence;       // Whether inside geofence
        float distance_to_center;   // Distance to center point (meters)
        
        // Accuracy radius
        float accuracy_radius;  // estimated error radius (meters)
    };

    // Geofence configuration
    struct Geofence {
        float center_latitude;    // Center point latitude
        float center_longitude;   // Center point longitude
        float radius;             // Geofence radius (meters)
        bool enabled;             // Whether geofence is enabled
        bool alarm_triggered;     // Whether alarm has been triggered
    };

    // Function declarations
    // Module initialization and main loop
    void localSetup();
    void localLoop();

    // Create JSON string from GPS data and send to server
    String createGPSJson(const GPSData &gps);
    void sendGPSData(const String &json_data, const String &serverUrl);

    // Parse NMEA sentence
    void parseNMEA(String nmea, GPSData &gps);

    // Display GPS information
    void displayGPSInfo(const GPSData &gps);

    // Calculate distance between two points
    float calculateDistance(float lat1, float lon1, float lat2, float lon2);

    // Check if within geofence
    void checkGeofence(GPSData &gps, Geofence &fence);

    // Set geofence
    void setGeofence(float lat, float lon, float radius, bool enabled);

    // Check if geofence alarm is triggered
    bool isGeofenceAlarmTriggered();

    // Reset geofence alarm to false
    void resetGeofenceAlarm();

    // Check if valid location is available
    bool hasValidLocation();

    // Get time since last valid fix
    unsigned long getTimeSinceLastValidFix();

    // Calculate and return current accuracy radius (meters)
    float getAccuracyRadius();

    extern const char* serverUrl;

    // External declarations for global variables
    extern String nmea_sentence;
    extern bool new_data;
    extern GPSData gps;
    extern Geofence geofence;


} // namespace gps