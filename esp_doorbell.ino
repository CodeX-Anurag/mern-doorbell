//PORT COM whatever
//BOARD: AI Thinker ESP32-CAM
//UPLOAD-SPEED: 115200
//PARTITION SCHEME: Huge App


#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

//
// === BOARD: AI Thinker ESP32-CAM ===
// Built-in pins: OV2640 camera + onboard flash LED
//
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

// === CONFIGURATION ===
#define BUTTON_PIN 13       // Push button input pin
#define FLASH_LED_PIN 4     // Onboard flash LED (GPIO 4)

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";
const char* serverUrl = "http://<PC_IP>:5000/upload"; // Replace with your backend IP

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // button connected to GND when pressed
  pinMode(FLASH_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);

  // ==== WiFi connection ====
  Serial.printf("Connecting to WiFi: %s\n", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // ==== Camera configuration ====
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
  config.pin_xclk = XCLK_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_VGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    return;
  }
  Serial.println("Camera initialized!");
}

void loop() {
  // Wait for button press
  if (digitalRead(BUTTON_PIN) == LOW) {
    delay(50); // debounce
    if (digitalRead(BUTTON_PIN) == LOW) {
      Serial.println("Button pressed! Capturing image...");
      digitalWrite(FLASH_LED_PIN, HIGH); // Turn ON flash

      // Capture image
      camera_fb_t* fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed!");
        digitalWrite(FLASH_LED_PIN, LOW);
        delay(1000);
        return;
      }

      // Send image to server
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverUrl);
        http.addHeader("Content-Type", "image/jpeg");

        int httpResponseCode = http.POST(fb->buf, fb->len);
        if (httpResponseCode > 0) {
          Serial.printf("Image sent successfully! Response code: %d\n", httpResponseCode);
        } else {
          Serial.printf("Error sending image: %s\n", http.errorToString(httpResponseCode).c_str());
        }
        http.end();
      } else {
        Serial.println("WiFi not connected!");
      }

      esp_camera_fb_return(fb);
      digitalWrite(FLASH_LED_PIN, LOW);
      delay(2000); // small cooldown
    }
  }
}
