#include "../include/camera.hpp"

namespace camera 
{
    const char* server = "http://3.8.78.228:8000/api/products/";


    String urlencode(String str)
    {
        String encodedString="";
        char c;
        char code0;
        char code1;
        for (int i = 0; i < str.length(); i++)
        {
            c = str.charAt(i);
            if (c == ' ')
            {
                encodedString += '+';
            } 
            else if (isalnum(c))
            {
                encodedString += c;
            } 
            else 
            {
                code1 = (c & 0xf) + '0';
                if ((c & 0xf) > 9){
                    code1 = (c & 0xf) - 10 + 'A';
                }
                c = (c >> 4) & 0xf;
                code0 = c + '0';
                if (c > 9){
                    code0 = c - 10 + 'A';
                }
                encodedString += '%';
                encodedString += code0;
                encodedString += code1;
            }
            yield();
        }
        return encodedString;
    }

    // camera setup
    void localSetup() 
    {
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        
        // initialize ws2812
        ws2812Init();

        // set color to green if camera init success
        if(initCamera() == 1) ws2812SetColor(2);

        // else set color to red if camera init failed
        else ws2812SetColor(1);
    }

    // camera loop
    void localLoop() 
    {
        // Serial.println("Camera loop");
        // if button is pressed
        if(digitalRead(BUTTON_PIN) == LOW)
        {
            delay(100);
            
            if(digitalRead(BUTTON_PIN) == LOW)
            {
                Serial.println("Button still pressed");

                ws2812SetColor(3);

                // wait for button to be released
                while(digitalRead(BUTTON_PIN) == LOW);
                Serial.println("Button released");
                // get image from camera
                camera_fb_t * fb = NULL;
                fb = esp_camera_fb_get();
                Serial.println("Image captured");

                // if image capture failed, print error message
                if(!fb) Serial.println("Camera capture failed");

                // if image capture success, send image to server
                else 
                {
                    // get image buffer
                    char *input = (char *)fb->buf;
                    char output[base64_enc_len(3)];
                    String imageFile = "";
                    
                    Serial.println("Encoding image to base64");
                    // encode image to base64
                    for (int i=0; i<fb->len; i++) 
                    {
                        base64_encode(output, (input++), 3);
                        if (i%3 == 0) imageFile += urlencode(String(output));
                    }

                    // send image to server
                    WiFiClient client;
                    HTTPClient http;
                    http.begin(client, server);

                    bool beginSuccess = http.begin(client, server);
                    
                    if (!beginSuccess) 
                    {
                        Serial.println("HTTP Begin failed! Unable to connect to server.");
                    } 
                    else 
                    {
                        Serial.println("HTTP Begin successful. Connection established.");
                    }

                    // add header to request
                    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

                    // print the size of the image
                    Serial.print("Image data size: ");
                    Serial.println(imageFile.length());

                    // send image to server
                    int httpResponseCode = http.POST("image=" + imageFile + "&id=1");
                    Serial.print("Camera HTTP Response code: ");
                    Serial.println(httpResponseCode);
                    
                    http.end();
                }

                esp_camera_fb_return(fb);
                ws2812SetColor(2);
            }
        }
    }


    int initCamera() 
    {
        // camera config
        camera_config_t config;
        config.ledc_channel = LEDC_CHANNEL_0;
        config.ledc_timer = LEDC_TIMER_0;
        config.pin_d0 = Y2_GPIO_NUM;
        config.pin_d1 = Y3_GPIO_NUM;
        config.pin_d2 = Y4_GPIO_NUM;
        config.pin_d3 = Y5_GPIO_NUM;
        config.pin_d4 = Y6_GPIO_NUM;
        config.pin_d5 = Y7_GPIO_NUM;
        config.pin_d6 = Y8_GPIO_NUM;
        config.pin_d7 = Y9_GPIO_NUM;
        config.pin_xclk = XCLK_GPIO_NUM;
        config.pin_pclk = PCLK_GPIO_NUM;
        config.pin_vsync = VSYNC_GPIO_NUM;
        config.pin_href = HREF_GPIO_NUM;
        config.pin_sscb_sda = SIOD_GPIO_NUM;
        config.pin_sscb_scl = SIOC_GPIO_NUM;
        config.pin_pwdn = PWDN_GPIO_NUM;
        config.pin_reset = RESET_GPIO_NUM;
        config.xclk_freq_hz = 20000000;
        config.frame_size = FRAMESIZE_UXGA;
        config.pixel_format = PIXFORMAT_JPEG; // for streaming
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.jpeg_quality = 4;
        config.fb_count = 1;
    
        // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
        // for larger pre-allocated frame buffer.
        if(psramFound())
        {
            config.jpeg_quality = 8;
            config.fb_count = 2;
            config.grab_mode = CAMERA_GRAB_LATEST;
        } 
        else 
        {
            // Limit the frame size when PSRAM is not available
            config.frame_size = FRAMESIZE_240X240;
            config.fb_location = CAMERA_FB_IN_DRAM;
        }

        // camera init
        esp_err_t err = esp_camera_init(&config);

        if (err != ESP_OK) 
        {
            Serial.printf("Camera init failed with error 0x%x", err);
            return 0;
        }

        sensor_t * s = esp_camera_sensor_get();
        // initial sensors are flipped vertically and colors are a bit saturated
        s->set_vflip(s, 1); // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, 0); // lower the saturation

        Serial.println("Camera configuration complete!");
        return 1;
    }

}