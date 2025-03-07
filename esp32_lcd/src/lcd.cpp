#include "lcd.hpp"

// WiFi connection information
const char* ssid = "CM";
const char* password = "akfbejevsj37jfb@";

// Server port
const int serverPort = 12000;

// Global variables
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
WiFiServer server(serverPort);

void setupDisplayServer() {
    // Initialize serial communication
    Serial.begin(115200);
    
    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting...");
    }
    
    // Print IP address after successful WiFi connection
    Serial.println("Connected to WiFi!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Start server
    server.begin();
    Serial.println("Server running on port 12000");

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
    if (client) {
        Serial.println("Client connected");
        
        // Receive data from client
        String message = "";
        while (client.available()) {
            char c = client.read();
            message += c;
        }
        Serial.print("Message received:");
        Serial.println(message);
        
        // If no space, print directly, otherwise call parse function
        int index = message.indexOf(" ");
        if (index == -1) {
            tft.println(message);
            message = "RECEIVED";
        }
        else {
            message = decode_msg(message.substring(0, index), message.substring(index + 1));
        }
        
        // Send response to client
        client.println(message);
        client.stop();
        Serial.println("Client disconnected");
    }
    delay(1000);
}

String decode_msg(String Keywd, String Payload) {
    if (Keywd == "Text_color") { // Set text color
        if (Payload == "red") {
            tft.setTextColor(ST77XX_RED);
            return "Success";
        }
        if (Payload == "blue") {
            tft.setTextColor(ST77XX_BLUE);
            return "Success";
        }
        if (Payload == "green") {
            tft.setTextColor(ST77XX_GREEN);
            return "Success";
        }
        if (Payload == "white") {
            tft.setTextColor(ST77XX_WHITE);
            return "Success";
        }
        if (Payload == "black") {
            tft.setTextColor(ST77XX_BLACK);
            return "Success";
        }
        if (Payload == "yellow") {
            tft.setTextColor(ST77XX_YELLOW);
            return "Success";
        }
        if (Payload == "orange") {
            tft.setTextColor(ST77XX_ORANGE);
            return "Success";
        }
        if (Payload == "magenta") {
            tft.setTextColor(ST77XX_MAGENTA);
            return "Success";
        }
        if (Payload == "cyan") {
            tft.setTextColor(ST77XX_CYAN);
            return "Success";
        }
        return "Invalid Color";
    }
  
    if (Keywd == "Fill_color") { // Set fill color and clear screen
        if (Payload == "red") {
            tft.fillScreen(ST77XX_RED);
            return "Success";
        }
        if (Payload == "blue") {
            tft.fillScreen(ST77XX_BLUE);
            return "Success";
        }
        if (Payload == "green") {
            tft.fillScreen(ST77XX_GREEN);
            return "Success";
        }
        if (Payload == "white") {
            tft.fillScreen(ST77XX_WHITE);
            return "Success";
        }
        if (Payload == "black") {
            tft.fillScreen(ST77XX_BLACK);
            return "Success";
        }
        if (Payload == "yellow") {
            tft.fillScreen(ST77XX_YELLOW);
            return "Success";
        }
        if (Payload == "orange") {
            tft.fillScreen(ST77XX_ORANGE);
            return "Success";
        }
        if (Payload == "magenta") {
            tft.fillScreen(ST77XX_MAGENTA);
            return "Success";
        }
        if (Payload == "cyan") {
            tft.fillScreen(ST77XX_CYAN);
            return "Success";
        }
        return "Invalid Color";
    }
  
    if (Keywd == "Rotation") { // Set screen rotation
        if (Payload == "0") {
            tft.setRotation(0);
            return "Success";
        }
        if (Payload == "1") {
            tft.setRotation(1);
            return "Success";
        }
        if (Payload == "2") {
            tft.setRotation(2);
            return "Success";
        }
        if (Payload == "3") {
            tft.setRotation(3);
            return "Success";
        }
    }
  
    if (Keywd == "Cursor") { // Set text starting position (x, y coordinates)
        int index_1 = Payload.indexOf(" ");
        String x, y;
        if (index_1 == -1) {
            return "Too few arguments (Needs 1 more)";
        }
        else {
            x = Payload.substring(0, index_1);
        }
        int index_2 = Payload.substring(index_1 + 1).indexOf(" ");
        if (index_2 == -1) {
            y = Payload.substring(index_1 + 1);
            tft.setCursor(x.toInt(), y.toInt());
            return "Success";
        }
        else {
            return "Too many arguments";
        }
    }
  
    if (Keywd == "Rect") { // Draw rectangle (top-left x, top-left y, width, height, color)
        String Params = Payload;
        String p, q, r, s, t;
        int index = Params.indexOf(" "); 
        if (index == -1) {
            return "Too few arguments (Needs 4 more)";
        }
        else {
            p = Params.substring(0, index);
            Params = Params.substring(index + 1);
        }
        index = Params.indexOf(" "); 
        if (index == -1) {
            return "Too few arguments (Needs 3 more)";
        }
        else {
            q = Params.substring(0, index);
            Params = Params.substring(index + 1);
        }
        index = Params.indexOf(" "); 
        if (index == -1) {
            return "Too few arguments (Needs 2 more)";
        }
        else {
            r = Params.substring(0, index);
            Params = Params.substring(index + 1);
        }
        index = Params.indexOf(" "); 
        if (index == -1) {
            return "Too few arguments (Needs 1 more)";
        }
        else {
            s = Params.substring(0, index);
            Params = Params.substring(index + 1);
        }
        index = Params.indexOf(" ");
        if (index == -1) {
            t = Params;
        
            if (t == "red") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_RED);
                return "Success";
            }
            if (t == "blue") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_BLUE);
                return "Success";
            }
            if (t == "green") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_GREEN);
                return "Success";
            }
            if (t == "white") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_WHITE);
                return "Success";
            }
            if (t == "black") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_BLACK);
                return "Success";
            }
            if (t == "yellow") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_YELLOW);
                return "Success";
            }
            if (t == "orange") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_ORANGE);
                return "Success";
            }
            if (t == "magenta") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_MAGENTA);
                return "Success";
            }
            if (t == "cyan") {
                tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_CYAN);
                return "Success";
            }
            return "Invalid Color";
        }
        else {
            Serial.println(Payload);
            return "Too many arguments";
        }
    }
    else { // If no specific keyword, print text directly
        tft.println(Keywd + " " + Payload);
        return "Success";
    }
    return "Failed";
}
