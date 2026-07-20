/*
  ปั้มลมไฟฟ้า - WiFi Manager + NTP Schedule
  เปิด 07:00 ปิด 18:00 (UTC+7 Thailand)
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// EEPROM addresses
#define EEPROM_SIZE 128
#define SSID_ADDR 0
#define PASS_ADDR 32
#define ON_HOUR_ADDR 96
#define ON_MIN_ADDR 97
#define OFF_HOUR_ADDR 98
#define OFF_MIN_ADDR 99

// Pin Configuration
#define RELAY_PIN 2  // GPIO2 (D4) - สวิตช์ปั้ม/ลม

// WiFi Settings
String wifiSSID = "";
String wifiPassword = "";

// Device State
bool relayState = false;
bool scheduleEnabled = false;

// Schedule Time (stored in EEPROM)
uint8_t onHour = 7, onMin = 0;
uint8_t offHour = 18, offMin = 0;

// NTP Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600, 60000); // UTC+7

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
  
  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  
  // Load WiFi credentials from EEPROM
  loadWiFiCredentials();
  loadScheduleTime();
  
  if (wifiSSID.length() > 0) {
    WiFi.begin(wifiSSID.c_str(), wifiPassword.c_str());
    Serial.print("Connecting to WiFi");
    
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());
      timeClient.begin();
      setupAllRoutes();  // Setup both config and device routes
    } else {
      Serial.println("\nWiFi connect failed, starting config portal");
      startConfigPortal();
    }
  } else {
    startConfigPortal();
  }
  
  server.begin();
}

void loop() {
  server.handleClient();
  
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    checkSchedule();
  }
}

void loadWiFiCredentials() {
  for (int i = 0; i < 32; i++) {
    char c = EEPROM.read(SSID_ADDR + i);
    if (c == 0) break;
    wifiSSID += c;
  }
  
  for (int i = 0; i < 64; i++) {
    char c = EEPROM.read(PASS_ADDR + i);
    if (c == 0) break;
    wifiPassword += c;
  }
}

void loadScheduleTime() {
  onHour = EEPROM.read(ON_HOUR_ADDR);
  onMin = EEPROM.read(ON_MIN_ADDR);
  offHour = EEPROM.read(OFF_HOUR_ADDR);
  offMin = EEPROM.read(OFF_MIN_ADDR);
  
  // Default to 07:00 / 18:00 if not set
  if (onHour == 0 || onHour > 23) onHour = 7;
  if (offHour == 0 || offHour > 23) offHour = 18;
}

void saveScheduleTime(uint8_t onH, uint8_t onM, uint8_t offH, uint8_t offM) {
  EEPROM.write(ON_HOUR_ADDR, onH);
  EEPROM.write(ON_MIN_ADDR, onM);
  EEPROM.write(OFF_HOUR_ADDR, offH);
  EEPROM.write(OFF_MIN_ADDR, offM);
  EEPROM.commit();
}

void saveWiFiCredentials(String ssid, String pass) {
  for (int i = 0; i < 32; i++) {
    EEPROM.write(SSID_ADDR + i, i < ssid.length() ? ssid[i] : 0);
  }
  for (int i = 0; i < 64; i++) {
    EEPROM.write(PASS_ADDR + i, i < pass.length() ? pass[i] : 0);
  }
  EEPROM.commit();
}

// WiFi Configuration Portal
void startConfigPortal() {
  WiFi.softAP("PumpFan_Setup", "12345678");
  IPAddress apIP(192, 168, 4, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  
  setupConfigRoutes();
  
  Serial.println("Config Portal started at http://192.168.4.1");
}

void setupConfigRoutes() {
  // เช็คว่าเป็น AP mode หรือ STA mode
  bool isApMode = (WiFi.status() != WL_CONNECTED);
  
  // Main config page (หน้าแรก)
  server.on("/", HTTP_GET, [isApMode]() {
    String currentIp = WiFi.softAPIP().toString();
    if (!isApMode) currentIp = WiFi.localIP().toString();
    
    String html = R"rawlPool(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ปั้มลมไฟฟ้า Dashboard</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 20px; background: #f5f5f5; }
    .card { background: white; padding: 20px; border-radius: 10px; margin-bottom: 16px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }
    h2 { color: #333; margin-top: 0; }
    .status { font-size: 18px; padding: 10px; background: #e8f4fd; border-radius: 5px; margin: 10px 0; }
    button { background: #007bff; color: white; padding: 12px 20px; border: none; border-radius: 5px; margin: 5px; cursor: pointer; }
    .success { background: #28a745; }
  </style>
</head>
<body>
  <div class="card">
    <h2>🔌 ปั้มลมไฟฟ้า</h2>
    <p>IP: )" + currentIp + R"rawlPool(</p>
    <div class="status">
      สถานะ: <b>กำลังตั้งค่า</b>
    </div>
  </div>
  <div class="card">
    <h3>ตั้งค่า WiFi</h3>
    <form action="/save" method="POST">
      <input type="text" name="ssid" placeholder="ชื่อ WiFi (SSID)" required><br>
      <input type="password" name="password" placeholder="รหส WiFi" required><br>
      <button type="submit">บันทึก WiFi</button>
    </form>
  </div>
</body>
</html>
)rawlPool";
    server.send(200, "text/html", html);
  });

  // API status endpoint
  server.on("/api/status", HTTP_GET, []() {
    String json = String("{\"relay\":") + (relayState ? "on" : "off") +
                  ",\"schedule\":{\"on\":\"" + String(onHour) + ":00\"" +
                  ",\"off\":\"" + String(offHour) + ":00\"" +
                  ",\"enabled\":") + (scheduleEnabled ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });

  // Save credentials
  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid");
    String pass = server.arg("password");
    
    if (ssid.length() > 0 && pass.length() > 0) {
      saveWiFiCredentials(ssid, pass);
      
      String html = R"rawlPool(
<!DOCTYPE html>
<html>
<head><meta name="viewport" content="width=device-width, initial-scale=1"></head>
<body style="font-family: Arial; text-align: center; padding: 50px;">
  <h2>✅ บันทึกสำเร็จ!</h2>
  <p>กรุณากดปุ่ม reset เพื่อเริ่มให้ทำงาน</p>
  <p style="color: red;">รอ 30 วินาที...</p>
</body>
</html>
)rawlPool";
      server.send(200, "text/html", html);
      
      delay(2000);
      ESP.restart();
    }
  });
}

// Device Control Routes
void setupAllRoutes() {
  setupConfigRoutes();
  setupDeviceRoutes();
}

void setupDeviceRoutes() {
  // Device status page
  server.on("/api/status", HTTP_GET, []() {
    String json = String("{\"relay\":") + (relayState ? "on" : "off") +
                  ",\"schedule\":{\"on\":\"" + String(onHour) + ":" + String(onMin) + "\"" +
                  ",\"off\":\"" + String(offHour) + ":" + String(offMin) + "\"" +
                  ",\"enabled\":" + (scheduleEnabled ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });

  // Toggle relay
  server.on("/api/relay/on", HTTP_GET, []() {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    server.send(200, "application/json", "{\"state\":\"on\"}");
  });
  
  server.on("/api/relay/off", HTTP_GET, []() {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    server.send(200, "application/json", "{\"state\":\"off\"}");
  });
  
  server.on("/api/relay/toggle", HTTP_GET, []() {
    relayState = !relayState;
    digitalWrite(RELAY_PIN, relayState ? HIGH : LOW);
    String json = String("{\"state\":\"") + (relayState ? "on" : "off") + "}";
    server.send(200, "application/json", json);
  });
  
  // Schedule routes - ตั้งเวลาเปิด
  server.on("/api/schedule/on/", HTTP_GET, []() {
    String path = server.uri();
    int hour = path.substring(path.lastIndexOf('/') + 1).toInt();
    int minute = 0; // Default
    
    if (hour >= 0 && hour <= 23) {
      onHour = hour;
      onMin = minute;
      EEPROM.write(ON_HOUR_ADDR, onHour);
      EEPROM.write(ON_MIN_ADDR, onMin);
      EEPROM.commit();
      server.send(200, "application/json", "{\"schedule_on\":\"" + String(hour) + ":00\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid hour\"}");
    }
  });

  // Schedule routes - ตั้งเวลาปิด
  server.on("/api/schedule/off/", HTTP_GET, []() {
    String path = server.uri();
    int hour = path.substring(path.lastIndexOf('/') + 1).toInt();
    int minute = 0; // Default
    
    if (hour >= 0 && hour <= 23) {
      offHour = hour;
      offMin = minute;
      EEPROM.write(OFF_HOUR_ADDR, offHour);
      EEPROM.write(OFF_MIN_ADDR, offMin);
      EEPROM.commit();
      server.send(200, "application/json", "{\"schedule_off\":\"" + String(hour) + ":00\"}");
    } else {
      server.send(400, "application/json", "{\"error\":\"Invalid hour\"}");
    }
  });

  // Enable/disable schedule
  server.on("/api/schedule/enable", HTTP_GET, []() {
    scheduleEnabled = true;
    server.send(200, "application/json", "{\"schedule_enabled\":true}");
  });

  server.on("/api/schedule/disable", HTTP_GET, []() {
    scheduleEnabled = false;
    server.send(200, "application/json", "{\"schedule_enabled\":false}");
  });
}

// Check schedule by real-time
void checkSchedule() {
  if (!scheduleEnabled) return;
  
  int currentHour = timeClient.getHours();
  int currentMin = timeClient.getMinutes();
  
  // เปิดที่เวลาที่ตั้ง
  if (currentHour == onHour && currentMin == onMin && !relayState) {
    digitalWrite(RELAY_PIN, HIGH);
    relayState = true;
    Serial.println("Scheduled ON at " + String(currentHour) + ":" + String(currentMin));
  }
  
  // ปิดที่เวลาที่ตั้ง
  if (currentHour == offHour && currentMin == offMin && relayState) {
    digitalWrite(RELAY_PIN, LOW);
    relayState = false;
    Serial.println("Scheduled OFF at " + String(currentHour) + ":" + String(currentMin));
  }
}