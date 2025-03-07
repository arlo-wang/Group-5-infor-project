#include "lcd.hpp"
#include <unordered_map>

// WiFi connection information
const char* ssid = "CM";
const char* password = "akfbejevsj37jfb@";

// Server port
const int serverPort = 12000;

// Global variables
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
WiFiServer server(serverPort);

// Store color mapping table in PROGMEM to save RAM
// Use unordered_map to improve lookup efficiency
const std::map<String, uint16_t> colorMap = {
    {"red",     ST77XX_RED},
    {"blue",    ST77XX_BLUE},
    {"green",   ST77XX_GREEN},
    {"white",   ST77XX_WHITE},
    {"black",   ST77XX_BLACK},
    {"yellow",  ST77XX_YELLOW},
    {"orange",  ST77XX_ORANGE},
    {"magenta", ST77XX_MAGENTA},
    {"cyan",    ST77XX_CYAN}
};

void setupDisplayServer() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Connect to WiFi
    Serial.println(F("Connecting to WiFi..."));
    
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println(F("Connecting..."));
    }
    
    // Print IP address after successful WiFi connection
    Serial.println(F("Connected to WiFi!"));
    Serial.print(F("IP Address: "));
    Serial.println(WiFi.localIP());
    
    // Start server
    server.begin();
    Serial.println(F("Server running on port 12000"));

    // Initialize display
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(0); // Adjust orientation
    
    // Clear screen and set text properties
    tft.fillScreen(ST77XX_BLACK);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
}

void loopDisplayServer() {
    // Check if a client has connected
    WiFiClient client = server.available();
    if (client) 
    {
        Serial.println(F("Client connected"));
        
        // Use a fixed-size buffer to receive data, avoiding dynamic memory allocation
        char buffer[128] = {0};
        size_t length = client.readBytesUntil('\n', buffer, sizeof(buffer) - 1);
        buffer[length] = '\0';
        
        String message(buffer);
        Serial.print(F("Message received:"));
        Serial.println(message);
        
        // If no space, print directly, otherwise call parse function
        int index = message.indexOf(" ");
        if (index == -1) 
        {
            tft.println(message);
            message = F("RECEIVED");
        }
        else 
        {
            message = decode_msg(message.substring(0, index), message.substring(index + 1));
        }
        
        // Send response to client
        client.println(message);
        client.stop();
        Serial.println(F("Client disconnected"));
    }
    delay(10); // Reduce delay time to improve response speed
}

// Helper function for color setting
bool setColor(const String& colorName, uint16_t& colorValue) 
{
    auto it = colorMap.find(colorName);
    if (it != colorMap.end()) 
    {
        colorValue = it->second;
        return true;
    }
    return false;
}

String decode_msg(String Keywd, String Payload) 
{
    // Handle text color setting
    if (Keywd == "Text_color") 
    {
        uint16_t color;
        if (setColor(Payload, color)) 
        {
            tft.setTextColor(color);
            return F("Success");
        }
        return F("Invalid Color");
    }
  
    // Handle fill color setting
    if (Keywd == "Fill_color") 
    {
        uint16_t color;
        if (setColor(Payload, color)) 
        {
            tft.fillScreen(color);
            return F("Success");
        }
        return F("Invalid Color");
    }
  
    // Handle screen rotation
    if (Keywd == "Rotation") 
    {
        int rotation = Payload.toInt();
        if (rotation >= 0 && rotation <= 3) 
        {
            tft.setRotation(rotation);
            return F("Success");
        }
        return F("Invalid Rotation (0-3)");
    }
  
    // Handle cursor position setting
    if (Keywd == "Cursor") 
    {
        int index = Payload.indexOf(" ");
        if (index == -1) 
        {
            return F("Too few arguments (Needs 1 more)");
        }
        
        int x = Payload.substring(0, index).toInt();
        String yStr = Payload.substring(index + 1);
        
        if (yStr.indexOf(" ") != -1) 
        {
            return F("Too many arguments");
        }
        
        int y = yStr.toInt();
        tft.setCursor(x, y);
        return F("Success");
    }
  
    // Handle rectangle drawing
    if (Keywd == "Rect") 
    {
        // This part could be further optimized using StringView or similar techniques
        int params[4]; // x, y, width, height
        int paramCount = 0;
        int startPos = 0;
        int spacePos;
        
        // Parse the first four numeric parameters
        while (paramCount < 4 && (spacePos = Payload.indexOf(' ', startPos)) != -1) 
        {
            params[paramCount++] = Payload.substring(startPos, spacePos).toInt();
            startPos = spacePos + 1;
        }
        
        // Check parameter count
        if (paramCount < 4) 
        {
            return F("Too few arguments");
        }
        
        // Get color name
        String colorName = Payload.substring(startPos);
        if (colorName.indexOf(' ') != -1) 
        {
            return F("Too many arguments");
        }
        
        // Set color and draw rectangle
        uint16_t color;
        if (setColor(colorName, color)) 
        {
            tft.fillRect(params[0], params[1], params[2], params[3], color);
            return F("Success");
        }
        return F("Invalid Color");
    }
    
    // If no matching keyword, print text directly
    tft.println(Keywd + " " + Payload);
    return F("Success");
}
