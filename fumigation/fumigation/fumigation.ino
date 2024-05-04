#include <HTTPClient.h>

const char* ssid = "HUEHUEHUE";
const char* password = "abc123456";
const char* SECRET_KEY = "7Xh#bl4Kt8}J#,zNAp%#QpzNEXJKQ";
const int system_id = 36;
const int FUMIGATION_ON_PIN = 21;
const int FUMIGATION_REQUEST_PIN = 22;
const int FUMIGATION_OFF_PIN = 23;
const int ULTRASONIC_TRIG_PIN = 18;
const int ULTRASONIC_ECHO_PIN = 19;

const int WATER_LEVEL_SUCCESS_PIN = 25;
const int WATER_LEVEL_REQUEST_PIN = 33;
const int WATER_LEVEL_ERROR_PIN = 32;

const int TRANSISTOR_SWITCH_PIN = 15;

#define MAX_DISTANCE 5 // Maximum distance for 100% water level full (in cm)
#define MIN_DISTANCE 30 // Minimum distance for 0% full (in cm)

void setup() {

  pinMode(FUMIGATION_ON_PIN, OUTPUT);
  pinMode(FUMIGATION_REQUEST_PIN, OUTPUT);
  pinMode(FUMIGATION_OFF_PIN, OUTPUT);
  pinMode(WATER_LEVEL_SUCCESS_PIN, OUTPUT);
  pinMode(WATER_LEVEL_REQUEST_PIN, OUTPUT);
  pinMode(WATER_LEVEL_ERROR_PIN, OUTPUT);
  pinMode(ULTRASONIC_TRIG_PIN, OUTPUT); // Set trigger pin as an output
  pinMode(ULTRASONIC_ECHO_PIN, INPUT); // Set echo pin as an input
  pinMode(TRANSISTOR_SWITCH_PIN, OUTPUT);

  offFumigation();
  onFumigationLeds();
  onWaterLevelLeds();
  
  Serial.begin(115200);
  Serial.print("Connecting to wifi");
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
}
void offFumigation(){
  digitalWrite(TRANSISTOR_SWITCH_PIN, HIGH);
}

void onFumigation(){
  digitalWrite(TRANSISTOR_SWITCH_PIN, LOW);
}

void resetFumigationLeds(){
  digitalWrite(FUMIGATION_ON_PIN, LOW);
  digitalWrite(FUMIGATION_REQUEST_PIN, LOW);
  digitalWrite(FUMIGATION_OFF_PIN, LOW);
}


void onFumigationLeds(){
  digitalWrite(FUMIGATION_ON_PIN, HIGH);
  digitalWrite(FUMIGATION_REQUEST_PIN, HIGH);
  digitalWrite(FUMIGATION_OFF_PIN, HIGH);
}

void resetWaterLevelLeds(){
  digitalWrite(WATER_LEVEL_SUCCESS_PIN, LOW);
  digitalWrite(WATER_LEVEL_REQUEST_PIN, LOW);
  digitalWrite(WATER_LEVEL_ERROR_PIN, LOW);
}

void onWaterLevelLeds(){
  digitalWrite(WATER_LEVEL_SUCCESS_PIN, HIGH);
  digitalWrite(WATER_LEVEL_REQUEST_PIN, HIGH);
  digitalWrite(WATER_LEVEL_ERROR_PIN, HIGH);
}

void sendWaterLevel(){
  
  digitalWrite(WATER_LEVEL_REQUEST_PIN, HIGH);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(ULTRASONIC_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(ULTRASONIC_TRIG_PIN, LOW);

  int duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  if(distance==0){
    digitalWrite(WATER_LEVEL_ERROR_PIN, HIGH);

    return;
  }

  int percentFull = map(distance, MAX_DISTANCE, MIN_DISTANCE, 100, 0);
  percentFull = constrain(percentFull, 0, 100); // Ensure the value is between 0 and 100
  
  HTTPClient http;
  String serverPath = "https://api.scale-anti-mosquito.site/v1/water_level/create";
  String payload = "{\"water_level\": \"" + String(percentFull)  + "\", \"secret_key\": \"" + SECRET_KEY + "\"}";
  Serial.print(payload);
  http.begin(serverPath.c_str());  //Specify request destination
  http.addHeader("Content-Type", "application/json");
  
  Serial.print("Percent Full: ");
  Serial.print(percentFull);
  Serial.println("%");

  int httpCode = http.sendRequest("POST", payload);
  Serial.println(httpCode);
  digitalWrite(WATER_LEVEL_REQUEST_PIN, LOW);
  if(httpCode == 201){
    digitalWrite(WATER_LEVEL_SUCCESS_PIN, HIGH);
    Serial.println("water level updated ");
  }else {
    digitalWrite(WATER_LEVEL_ERROR_PIN, HIGH);
    Serial.println("failed to post ");
  }

  Serial.println(http.getString());

  http.end();
}

void fetchShouldFumigate(){
    HTTPClient http;

    Serial.println("Starting request...");
    digitalWrite(FUMIGATION_REQUEST_PIN, HIGH);
    String serverPath = "https://api.scale-anti-mosquito.site/v1/system/" + String(system_id) + "/fumigate";
    http.begin(serverPath.c_str());  //Specify request destination
    int httpResponseCode = http.GET();


    if(httpResponseCode>0){
      digitalWrite(FUMIGATION_REQUEST_PIN, LOW);
      String response = http.getString();
      if(response=="1"){
        digitalWrite(FUMIGATION_ON_PIN, HIGH);
        onFumigation();
        delay(2000);
      }
      else{
        offFumigation();
        digitalWrite(FUMIGATION_OFF_PIN, HIGH);
      }
      offFumigation();
      Serial.println(httpResponseCode);
//      Serial.println(response); 
    }
    else{
      digitalWrite(FUMIGATION_OFF_PIN, HIGH);
      Serial.println("error");
      Serial.println(httpResponseCode); 
    }

    http.end();  //Close connection
}


void loop() {
  offFumigation();
  resetFumigationLeds();
  resetWaterLevelLeds();
  if(WiFi.status()== WL_CONNECTED){
    sendWaterLevel();
    fetchShouldFumigate();
  } else {
    onFumigationLeds();
    onWaterLevelLeds();
    
  }

  
//  delay(1000);

  
}
