 #include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "base64.h"

#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define LED_GPIO_NUM       4

#define DETECTION_ERROR_PIN 13
#define DETECTION_REQUEST_PIN 12
#define DETECTION_SUCCESS_PIN 15

// ===========================
// Enter your WiFi credentials
// ===========================
const char* ssid = "HUEHUEHUE";
const char* password = "abc123456";
const char *url = "https://api.scale-anti-mosquito.site/v1/mosquito/create"; //&image_only=true
const char *secret_key = "7Xh#bl4Kt8}J#,zNAp%#QpzNEXJKQ";

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

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
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_XGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  pinMode(DETECTION_ERROR_PIN, OUTPUT);
  pinMode(DETECTION_REQUEST_PIN, OUTPUT);
  pinMode(DETECTION_SUCCESS_PIN, OUTPUT);

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    while(true){
      onDetectionLeds();
      delay(250);
      resetDetectionLeds();
    }
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }



  onDetectionLeds();
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("Connecting to wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi connected");
  resetDetectionLeds();

//  pinMode(LED_GPIO_NUM, OUTPUT);
//  for (int brightness = 0; brightness <= 1; brightness++) {
//    analogWrite(LED_GPIO_NUM, brightness); // Set the LED brightness
//    delay(1); // Delay to observe the change in brightness
//  }

  
}

void onDetectionLeds(){
  digitalWrite(DETECTION_ERROR_PIN, HIGH);
  digitalWrite(DETECTION_REQUEST_PIN, HIGH);
  digitalWrite(DETECTION_SUCCESS_PIN, HIGH);
}

void resetDetectionLeds(){
  digitalWrite(DETECTION_ERROR_PIN, LOW);
  digitalWrite(DETECTION_REQUEST_PIN, LOW);
  digitalWrite(DETECTION_SUCCESS_PIN, LOW);
}

void uploadDetectionData(){
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if(fb==NULL){
    digitalWrite(DETECTION_ERROR_PIN, HIGH);
  }

  digitalWrite(DETECTION_REQUEST_PIN, HIGH);
  Serial.println("starting request");
  HTTPClient http;
  http.begin(url); //HTTP
  http.setTimeout(5000);
  String base64Image = base64::encode(fb->buf, fb->len);
  esp_camera_fb_return(fb);
 
  String payload = "{\"image\": \"" + base64Image  + "\", \"secret_key\": \"" + secret_key + "\"}";

  http.addHeader("Content-Type", "application/json");

  int httpCode = http.sendRequest("POST", payload);
  digitalWrite(DETECTION_REQUEST_PIN, LOW);
  if (httpCode == HTTP_CODE_CREATED ){
    Serial.println("sucessfully created");
    digitalWrite(DETECTION_SUCCESS_PIN, HIGH);
  } else {
    Serial.println("Error has occurred");
    Serial.println(httpCode);
    Serial.println(http.getString() );
    digitalWrite(DETECTION_ERROR_PIN, HIGH);
  }
  http.end();

  delay(500);
//  if (httpCode > 0)
//  {
//    // HTTP header has been send and Server response header has been handled
//    Serial.printf("[HTTP] POST... code: %d\n", httpCode);
//
//    // file found at server
//    if (httpCode == HTTP_CODE_CREATED )
//    {
//
//      WiFiClient stream = http.getStream();
//          size_t len = stream.available();
//          uint8_t* buffer = new uint8_t[len];
//          stream.readBytes(buffer, len);
//          Serial.write(buffer, len);
//
//    }
//  }
//  else
//  {
//    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
//  }
  
}

void loop() {
  resetDetectionLeds();

  if(WiFi.status()== WL_CONNECTED){
    uploadDetectionData();
  } else {
    onDetectionLeds();
  }

  return;
  

  

}
