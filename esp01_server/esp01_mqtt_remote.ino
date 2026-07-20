/*
  ESP01 MQTT Client - Remote Control via Adafruit IO
  ทำให้ควบคุมได้จากทุกที่ทั่วโลก!
*/

#include <ESP8266WiFi.h>
#include <AdafruitIO_WiFi.h>
#include <DHT.h> // ถ้ามี Sensor

// WiFi Credentials
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Adafruit IO Credentials (จากหน้า io.adafruit.com)
#define AIO_USERNAME "YOUR_ADAFRUIT_IO_USERNAME"
#define AIO_KEY "YOUR_ADAFRUIT_IO_KEY"

// Pin Configuration
#define PUMP_PIN     0  // GPIO0 (D3) - ปั้มน้ำ
#define FAN_PIN      2  // GPIO2 (D4) - ลมกัน

// Adafruit IO
AdafruitIO_WiFi io(AIO_USERNAME, AIO_KEY, WIFI_SSID, WIFI_PASSWORD);

// Feeds
AdafruitIO_Feed *pumpCtrl = io.feed("pump");
AdafruitIO_Feed *fanCtrl = io.feed("fan");
AdafruitIO_Feed *statusFeed = io.feed("status");

bool pumpState = false;
bool fanState = false;

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");
  
  // Setup Adafruit IO
  io.connect();
  pumpCtrl->onMessage(handlePumpMessage);
  fanCtrl->onMessage(handleFanMessage);
  
  // Wait for connection
  while (io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("\nAdafruit IO Connected!");
}

// Handle Pump Control Message
void handlePumpMessage(AdafruitIO_Data *data) {
  Serial.println("Received pump: " + String(data->value()));
  
  if (String(data->value()) == "ON") {
    digitalWrite(PUMP_PIN, HIGH);
    pumpState = true;
  } else if (String(data->value()) == "OFF") {
    digitalWrite(PUMP_PIN, LOW);
    pumpState = false;
  }
  
  // Send status back
  sendStatus();
}

// Handle Fan Control Message
void handleFanMessage(AdafruitIO_Data *data) {
  Serial.println("Received fan: " + String(data->value()));
  
  if (String(data->value()) == "ON") {
    digitalWrite(FAN_PIN, HIGH);
    fanState = true;
  } else if (String(data->value()) == "OFF") {
    digitalWrite(FAN_PIN, LOW);
    fanState = false;
  }
  
  sendStatus();
}

// Send status to Adafruit IO
void sendStatus() {
  String status = String("{\"pump\":") + (pumpState ? "ON" : "OFF") + 
                  String(",\"fan\":") + (fanState ? "ON" : "OFF") + "}";
  statusFeed->save(status.c_str());
}

void loop() {
  io.run(); // Keep connection alive
  
  // ส่งสถานะทุก 30 วินาที (ถ้าต้องการ)
  static unsigned long lastSend = 0;
  if (millis() - lastSend > 30000) {
    sendStatus();
    lastSend = millis();
  }
}