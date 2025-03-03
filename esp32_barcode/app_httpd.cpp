#include <Arduino.h>
#include "esp_http_server.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "camera_index.h"

extern camera_fb_t *last_captured_fb;  // Use the captured image from scanner.ino
httpd_handle_t camera_httpd = NULL;

// ---------------------------
// Handle Captured Image Display
// ---------------------------
static esp_err_t capture_handler(httpd_req_t *req) {
    if (!last_captured_fb) {
        Serial.println("❌ No barcode image available. Sending empty response.");
        httpd_resp_set_status(req, "204 No Content");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }

    Serial.println("📤 Sending last captured barcode image...");
    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    return httpd_resp_send(req, (const char *)last_captured_fb->buf, last_captured_fb->len);
}


// ---------------------------
// Webpage HTML
// ---------------------------
static esp_err_t index_handler(httpd_req_t *req) {
    const char html[] = "<html><head><title>Barcode Scanner</title></head>"
                        "<body><h1>Last Captured Barcode</h1>"
                        "<img src='/capture' style='max-width:100%' />"
                        "</body></html>";
    httpd_resp_set_type(req, "text/html");
    return httpd_resp_send(req, html, strlen(html));
}

// ---------------------------
// Start Web Server
// ---------------------------
void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_uri_t index_uri = { .uri = "/", .method = HTTP_GET, .handler = index_handler };
    httpd_uri_t capture_uri = { .uri = "/capture", .method = HTTP_GET, .handler = capture_handler };

    if (httpd_start(&camera_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(camera_httpd, &index_uri);
        httpd_register_uri_handler(camera_httpd, &capture_uri);
    }
}
