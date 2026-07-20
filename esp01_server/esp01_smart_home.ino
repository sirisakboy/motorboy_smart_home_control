/*
  ESP01 HTTP Server for Smart Home Control
  ควบคุมปั้มน้ำ ลมกัน พร้อมระบบ Delay/Timer
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi Credentials
const char* ssid = "4G-CPE-5C88";
const char* password = "1234567890";

// Pin Configuration
#define PUMP_PIN     0  // GPIO0 (D3) - ปั้มน้ำ
#define FAN_PIN      2  // GPIO2 (D4) - ลมกัน

// Device States
bool pumpState = false;
bool fanState = false;

// Schedule Settings
unsigned long pumpDelayStart = 0;
unsigned long fanDelayStart = 0;
int pumpDelayMinutes = 0;
int fanDelayMinutes = 0;
bool pumpAutoOff = false;
bool fanAutoOff = false;

ESP8266WebServer server(80);

void handleRoot() {
  String html = "<html><body>"
                "<h1>ESP01 Smart Home API</h1>"
                "<p>Use endpoints: /pump/on, /pump/off, /fan/on, /fan/off</p>"
                "<p>Set delay: /pump/delay/{minutes}, /fan/delay/{minutes}</p>"
                "</body></html>";
  server.send(200, "text/html", html);
}

// Pump Control Endpoints
void handlePumpOn() {
  digitalWrite(PUMP_PIN, HIGH);
  pumpState = true;
  pumpAutoOff = false;
  server.send(200, "application/json", "{\"device\":\"pump\",\"state\":\"on\",\"auto_off\":false}");
  Serial.println("Pump ON");
}

void handlePumpOff() {
  digitalWrite(PUMP_PIN, LOW);
  pumpState = false;
  pumpAutoOff = false;
  server.send(200, "application/json", "{\"device\":\"pump\",\"state\":\"off\",\"auto_off\":false}");
  Serial.println("Pump OFF");
}

void handlePumpStatus() {
  String json = "{\"device\":\"pump\",\"state\":" + String(pumpState ? "on" : "off") + 
                ",\"delay_remaining\":" + String(getRemainingTime(pumpDelayStart, pumpAutoOff)) + "}";
  server.send(200, "application/json", json);
}

void handlePumpDelay() {
  String path = server.uri();
  int minutes = path.substring(path.lastIndexOf('/') + 1).toInt();
  
  if (minutes > 0) {
    pumpDelayMinutes = minutes;
    pumpDelayStart = millis() + (minutes * 60 * 1000);
    pumpAutoOff = true;
    digitalWrite(PUMP_PIN, HIGH);
    pumpState = true;
  }
  
  String json = "{\"device\":\"pump\",\"state\":\"on\",\"delay_minutes\":" + String(minutes) + 
                ",\"auto_off\":true}";
  server.send(200, "application/json", json);
  Serial.println("Pump ON with " + String(minutes) + " min delay");
}

// Fan Control Endpoints
void handleFanOn() {
  digitalWrite(FAN_PIN, HIGH);
  fanState = true;
  fanAutoOff = false;
  server.send(200, "application/json", "{\"device\":\"fan\",\"state\":\"on\",\"auto_off\":false}");
  Serial.println("Fan ON");
}

void handleFanOff() {
  digitalWrite(FAN_PIN, LOW);
  fanState = false;
  fanAutoOff = false;
  server.send(200, "application/json", "{\"device\":\"fan\",\"state\":\"off\",\"auto_off\":false}");
  Serial.println("Fan OFF");
}

void handleFanStatus() {
  String json = "{\"device\":\"fan\",\"state\":" + String(fanState ? "on" : "off") + 
                ",\"delay_remaining\":" + String(getRemainingTime(fanDelayStart, fanAutoOff)) + "}";
  server.send(200, "application/json", json);
}

void handleFanDelay() {
  String path = server.uri();
  int minutes = path.substring(path.lastIndexOf('/') + 1).toInt();
  
  if (minutes > 0) {
    fanDelayMinutes = minutes;
    fanDelayStart = millis() + (minutes * 60 * 1000);
    fanAutoOff = true;
    digitalWrite(FAN_PIN, HIGH);
    fanState = true;
  }
  
  String json = "{\"device\":\"fan\",\"state\":\"on\",\"delay_minutes\":" + String(minutes) + 
                ",\"auto_off\":true}";
  server.send(200, "application/json", json);
  Serial.println("Fan ON with " + String(minutes) + " min delay");
}

// Auto-off check
void checkAutoOff() {
  if (pumpAutoOff && millis() >= pumpDelayStart) {
    digitalWrite(PUMP_PIN, LOW);
    pumpState = false;
    pumpAutoOff = false;
    Serial.println("Pump AUTO OFF");
  }
  
  if (fanAutoOff && millis() >= fanDelayStart) {
    digitalWrite(FAN_PIN, LOW);
    fanState = false;
    fanAutoOff = false;
    Serial.println("Fan AUTO OFF");
  }
}

int getRemainingTime(unsigned long targetTime, bool active) {
  if (!active) return 0;
  unsigned long remaining = targetTime - millis();
  return remaining > 0 ? remaining / 60000 : 0; // Return minutes
}

void setup() {
  Serial.begin(115200);
  
  // Initialize pins
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected! IP: " + WiFi.localIP().toString());
  
  // Setup API routes
  server.on("/", HTTP_GET, handleRoot);
  
  // Pump routes
  server.on("/pump/on", HTTP_GET, handlePumpOn);
  server.on("/pump/off", HTTP_GET, handlePumpOff);
  server.on("/pump/status", HTTP_GET, handlePumpStatus);
  server.on("/pump/delay/", HTTP_GET, handlePumpDelay);
  
  // Fan routes
  server.on("/fan/on", HTTP_GET, handleFanOn);
  server.on("/fan/off", HTTP_GET, handleFanOff);
  server.on("/fan/status", HTTP_GET, handleFanStatus);
  server.on("/fan/delay/", HTTP_GET, handleFanDelay);
  
  // All devices status
  server.on("/status", HTTP_GET, []() {
    String json = "{"
                  "\"pump\":{\"state\":" + String(pumpState ? "on" : "off") + ",\"delay\":" + String(pumpDelayMinutes) + "},"
                  "\"fan\":{\"state\":" + String(fanState ? "on" : "off") + ",\"delay\":" + String(fanDelayMinutes) + "}"
                  "}";
    server.send(200, "application/json", json);
  });
  
  server.begin();
  Serial.println("HTTP Server started");
}

void loop() {
  server.handleClient();
  checkAutoOff();
}