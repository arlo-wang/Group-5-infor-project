#pragma once

#include <Arduino.h>
#include <math.h>  // For distance calculation

// Serial port definitions
#define GPSSerial Serial2
#define DEBUGSerial Serial

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
    void parseNMEA(String nmea, GPSData &gps);
    void displayGPSInfo(const GPSData &gps);
    float calculateDistance(float lat1, float lon1, float lat2, float lon2);
    void checkGeofence(GPSData &gps, Geofence &fence);
    void setGeofence(float lat, float lon, float radius, bool enabled);
    bool isGeofenceAlarmTriggered();
    void resetGeofenceAlarm();
    bool hasValidLocation();
    unsigned long getTimeSinceLastValidFix();

    // External declarations for global variables
    extern String nmea_sentence;
    extern bool new_data;
    extern GPSData gps;
    extern Geofence geofence;

    // Module initialization and main loop
    void localSetup();
    void localLoop();
} // namespace gps