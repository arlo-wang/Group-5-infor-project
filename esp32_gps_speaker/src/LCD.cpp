#include "../include/LCD.hpp"

namespace LCD {
    const int port = 12000;
    WiFiServer server(serverPort);

    void localSetup()
    {
        tft.initR(INITR_BLACKTAB);
        tft.setRotation(0);
        tft.fillScreen(ST77XX_BLACK);
        tft.setTextColor(ST77XX_WHITE); 
        tft.setTextSize(1);
        tft.setCursor(0, 0); 
    }

    void localLoop()
    {
        WiFiClient client = server.available();
        if (client) {
            // tft.fillScreen(ST77XX_BLACK);
            Serial.println("Client connected");
            
            // Wait for data from client
            String message = "";
            while (client.available()) {
                char c = client.read();
                message += c;
            }
            Serial.print("Message received:");
            Serial.println(message);  

            // check instruction passed
            int index = message.indexOf(" ");
            if (index == -1){
                tft.println(message);
                message = "RECEIVED";
            }
            else {
                message = decode_msg(message.substring(0, index), message.substring(index + 1, message.length()));
            }

            client.println(message);
            
            client.stop();
            Serial.println("Client disconnected");
        }
    }

    String decode_msg(String Keywd, String Payload)
    {
        //sets text color
        if (Keywd == "Text_color"){ 
            if (Payload == "red"){
                tft.setTextColor(ST77XX_RED);
                return "Success";
            }
            if (Payload == "blue"){
                tft.setTextColor(ST77XX_BLUE);
                return "Success";
            }
            if (Payload == "green"){
                tft.setTextColor(ST77XX_GREEN);
                return "Success";
            }
            if (Payload == "white"){
                tft.setTextColor(ST77XX_WHITE);
                return "Success";
            }
            if (Payload == "black"){
                tft.setTextColor(ST77XX_BLACK);
                return "Success";
            }
            if (Payload == "yellow"){
                tft.setTextColor(ST77XX_YELLOW);
                return "Success";
            }
            if (Payload == "orange"){
                tft.setTextColor(ST77XX_ORANGE);
                return "Success";
            }
            if (Payload == "magenta"){
                tft.setTextColor(ST77XX_MAGENTA);
                return "Success";
            }
            if (Payload == "cyan"){
                tft.setTextColor(ST77XX_CYAN);
                return "Success";
            }
            return "Invalid Color";
        }

        //sets fill_color (also wipes screen)
        if (Keywd == "Fill_color")
        {  
            if (Payload == "red"){
                tft.fillScreen(ST77XX_RED);
                return "Success";
            }
            if (Payload == "blue"){
                tft.fillScreen(ST77XX_BLUE);
                return "Success";
            }
            if (Payload == "green"){
                tft.fillScreen(ST77XX_GREEN);
                return "Success";
            }
            if (Payload == "white"){
                tft.fillScreen(ST77XX_WHITE);
                return "Success";
            }
            if (Payload == "black"){
                tft.fillScreen(ST77XX_BLACK);
                return "Success";
            }
            if (Payload == "yellow"){
                tft.fillScreen(ST77XX_YELLOW);
                return "Success";
            }
            if (Payload == "orange"){
                tft.fillScreen(ST77XX_ORANGE);
                return "Success";
            }
            if (Payload == "magenta"){
                tft.fillScreen(ST77XX_MAGENTA);
                return "Success";
            }
            if (Payload == "cyan"){
                tft.fillScreen(ST77XX_CYAN);
                return "Success";
            }
            return "Invalid Color";
        }

        //sets the screen rotation
        if (Keywd == "Rotation") 
        { 
            if (Payload == "0"){
                tft.setRotation(0);
                return "Success";
            }
            if (Payload == "1"){
                tft.setRotation(1);
                return "Success";
            }
            if (Payload == "2"){
                tft.setRotation(2);
                return "Success";
            }
            if (Payload == "3"){
                tft.setRotation(3);
                return "Success";
            }
        }

        //change where text starts printing ((x, y) coords)
        if (Keywd == "Cursor") { 
            int index_1 = Payload.indexOf(" ");
            String x, y;
            if (index_1 == -1){
                return "Too few arguments (Needs 1 more)";
            }
            else {
                x = Payload.substring(0, index_1);
            }
            int index_2 = Payload.substring(index_1 + 1, Payload.length()).indexOf(" ");
            if (index_2 == -1) {
                y = Payload.substring(index_1 + 1, Payload.length());
                tft.setCursor(x.toInt(), y.toInt());
                return "Success";
            }
            else {
                return "Too many arguments";
            }
        }

        //print a rectangle (Top_left x, Top_left y, width, height, color)
        if (Keywd == "Rect") 
        { 
            String Params = Payload;
            String p, q, r, s, t;
            int index = Params.indexOf(" "); 
            if (index == -1) {
                return "Too few arguments (Needs 4 more)";
            }
            else {
                p = Params.substring(0, index);
                Params = Params.substring(index + 1, Params.length());
            }
            index = Params.indexOf(" "); 
            if (index == -1){
                return "Too few arguments (Needs 3 more)";
            }
            else {
                q = Params.substring(0, index);
                Params = Params.substring(index + 1, Params.length());
            }
            index = Params.indexOf(" "); 
            if (index == -1){
                return "Too few arguments (Needs 2 more)";
            }
            else {
                r = Params.substring(0, index);
                Params = Params.substring(index + 1, Params.length());
            }
            index = Params.indexOf(" "); 
            if (index == -1){
                return "Too few arguments (Needs 1 more)";
            }
            else {
                s = Params.substring(0, index);
                Params = Params.substring(index + 1, Params.length());
            }
            index = Params.indexOf(" ");
            if (index == -1){
                t = Params;
                if (t == "red"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_RED);
                    return "Success";
                }
                if (t == "blue"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_BLUE);
                    return "Success";
                }
                if (t == "green"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_GREEN);
                    return "Success";
                }
                if (t == "white"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_WHITE);
                    return "Success";
                }
                if (t == "black"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_BLACK);
                    return "Success";
                }
                if (t == "yellow"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_YELLOW);
                    return "Success";
                }
                if (t == "orange"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_ORANGE);
                    return "Success";
                }
                if (t == "magenta"){
                    tft.fillRect(p.toInt(), q.toInt(), r.toInt(), s.toInt(), ST77XX_MAGENTA);
                    return "Success";
                }
                if (t == "cyan"){
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

        else { //if no keyword assume text
            tft.println(Keywd + " " + Payload);
            return "Success";
        }

        return "Failed";
      }


} // namespace wifi