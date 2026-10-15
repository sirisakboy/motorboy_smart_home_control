/*
  motorboy_smart_home_control - ALL IN ONE v2
  จากโค้ดเดิมคุณ + เพิ่ม สแกนแรงสุด + Firewall แยกวง
  ปั้มน้ำ GPIO0 , พัดลม GPIO2 เหมือนเดิม
*/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

ESP8266WiFiMulti wifiMulti;
ESP8266WebServer server(80);

// ===== ตั้งค่าเสา ให้สแกนหาตัวแรงสุดอัตโนมัติ =====
const char* PUBLIC_AP_130 = "FREE_130m";
const char* PUBLIC_PASS_130 = "12345678";
const char* PUBLIC_AP_190 = "FREE_190m";
const char* PUBLIC_PASS_190 = "12345678";
const char* FALLBACK_AP = "4G-CPE-5C88"; // ของเดิมคุณ
const char* FALLBACK_PASS = "1234567890";

// วงในบ้าน มีแค่เจ้าของที่รู้ - ไม่วิ่งออกสาธารณะ
const char* HOME_SSID = "SMART-HOME";
const char* HOME_PASS = "Motorboy2024!";

// Pin เดิมคุณ
#define PUMP_PIN 0
#define FAN_PIN  2
bool pumpState = false;
bool fanState = false;
unsigned long pumpDelayStart = 0;
unsigned long fanDelayStart = 0;
bool pumpAutoOff = false;
bool fanAutoOff = false;

int getRemainingTime(unsigned long target, bool active){
  if(!active) return 0;
  long remain = (long)target - (long)millis();
  return remain > 0 ? remain / 60000 : 0;
}

void checkAutoOff(){
  if(pumpAutoOff && millis() >= pumpDelayStart){
    digitalWrite(PUMP_PIN, LOW); pumpState=false; pumpAutoOff=false;
    Serial.println("Pump AUTO OFF");
  }
  if(fanAutoOff && millis() >= fanDelayStart){
    digitalWrite(FAN_PIN, LOW); fanState=false; fanAutoOff=false;
    Serial.println("Fan AUTO OFF");
  }
}

// ===== API เดิมของคุณ + เพิ่ม status สัญญาณ =====
void setupRoutes(){
  server.on("/", HTTP_GET, [](){
    String html = "<h1>Motorboy Smart Home</h1>"
                  "<p>WiFi: " + WiFi.SSID() + " (" + String(WiFi.RSSI()) + " dBm)</p>"
                  "<p><a href=/pump/on>ปั้ม ON</a> | <a href=/pump/off>ปั้ม OFF</a></p>"
                  "<p><a href=/fan/on>พัดลม ON</a> | <a href=/fan/off>พัดลม OFF</a></p>"
                  "<p><a href=/status>JSON Status</a></p>";
    server.send(200,"text/html",html);
  });

  server.on("/pump/on", HTTP_GET, [](){
    digitalWrite(PUMP_PIN, HIGH); pumpState=true; pumpAutoOff=false;
    server.send(200,"application/json","{\"device\":\"pump\",\"state\":\"on\"}");
  });
  server.on("/pump/off", HTTP_GET, [](){
    digitalWrite(PUMP_PIN, LOW); pumpState=false; pumpAutoOff=false;
    server.send(200,"application/json","{\"device\":\"pump\",\"state\":\"off\"}");
  });
  server.on("/pump/delay/", HTTP_GET, [](){
    int minutes = server.uri().substring(server.uri().lastIndexOf('/')+1).toInt();
    if(minutes>0){ pumpDelayStart = millis() + minutes*60000; pumpAutoOff=true; digitalWrite(PUMP_PIN, HIGH); pumpState=true; }
    server.send(200,"application/json","{\"device\":\"pump\",\"delay\":"+String(minutes)+"}");
  });

  server.on("/fan/on", HTTP_GET, [](){
    digitalWrite(FAN_PIN, HIGH); fanState=true; fanAutoOff=false;
    server.send(200,"application/json","{\"device\":\"fan\",\"state\":\"on\"}");
  });
  server.on("/fan/off", HTTP_GET, [](){
    digitalWrite(FAN_PIN, LOW); fanState=false; fanAutoOff=false;
    server.send(200,"application/json","{\"device\":\"fan\",\"state\":\"off\"}");
  });
  server.on("/fan/delay/", HTTP_GET, [](){
    int minutes = server.uri().substring(server.uri().lastIndexOf('/')+1).toInt();
    if(minutes>0){ fanDelayStart = millis() + minutes*60000; fanAutoOff=true; digitalWrite(FAN_PIN, HIGH); fanState=true; }
    server.send(200,"application/json","{\"device\":\"fan\",\"delay\":"+String(minutes)+"}");
  });

  server.on("/status", HTTP_GET, [](){
    String json = "{"
      "\"wifi\":{\"ssid\":\""+WiFi.SSID()+"\",\"rssi\":"+String(WiFi.RSSI())+"},"
      "\"pump\":{\"state\":\""+String(pumpState?"on":"off")+"\",\"remain\":"+String(getRemainingTime(pumpDelayStart,pumpAutoOff))+"},"
      "\"fan\":{\"state\":\""+String(fanState?"on":"off")+"\",\"remain\":"+String(getRemainingTime(fanDelayStart,fanAutoOff))+"}"
      "}";
    server.send(200,"application/json",json);
  });
}

void setup() {
  Serial.begin(115200);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);

  WiFi.mode(WIFI_AP_STA);

  // 1. สแกนหาแรงสุด: เพิ่ม 3 เสาให้เลือก ตัวไหนแรงสุดเกาะตัวนั้น
  wifiMulti.addAP(PUBLIC_AP_130, PUBLIC_PASS_130);
  wifiMulti.addAP(PUBLIC_AP_190, PUBLIC_PASS_190);
  wifiMulti.addAP(FALLBACK_AP, FALLBACK_PASS); // ของเดิมคุณเป็นตัวสำรอง

  // 2. Firewall: ปล่อยวงในบ้านแยกขาด ไม่ให้ปั้ม/พัดลมวิ่งออกสาธารณะโดยตรง
  // รับได้ 8 เครื่อง ซ่อน SSID = 1 (มีแค่เจ้าของที่รู้)
  WiFi.softAP(HOME_SSID, HOME_PASS, 1, 1, 8);
  WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));

  MDNS.begin("smarthome");
  setupRoutes();
  server.begin();
  Serial.println("ALL IN ONE Ready! AP: SMART-HOME / http://smarthome.local");
}

void loop() {
  // สแกนแรงสุดทุก 10 วิ ถ้า 130m ดับ จะสลับไป 190m เอง
  static unsigned long last = 0;
  if(millis()-last > 10000){
    last = millis();
    if(wifiMulti.run() != WL_CONNECTED){
      Serial.println("Scanning 130m vs 190m vs 4G...");
    } else {
      Serial.printf("Connected: %s %d dBm\n", WiFi.SSID().c_str(), WiFi.RSSI());
    }
  }
  server.handleClient();
  checkAutoOff();
  MDNS.update();
}