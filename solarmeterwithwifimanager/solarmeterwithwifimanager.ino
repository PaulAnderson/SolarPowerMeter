#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <TM1637Display.h>
#include <WiFiManager.h>
#include <Ticker.h>


#define USE_SERIAL Serial

#define CLK 4  //D2
#define DIO 5  //D1

const int sleepSeconds = 1;

const String url = "/solar_api/v1/GetInverterRealtimeData.fcgi?Scope=System";
String fullUrl = "";
int retryCount = 0;

WiFiManager wifiManager;
ESP8266WiFiMulti WiFiMulti;
Ticker ticker;

TM1637Display display(CLK, DIO);

void showNum(int n)
{
  display.setBrightness(0x0f); //need this or nothing displayed
  display.showNumberDec(n, false, 4, 0);
}

void showNumDim(int n)
{
  display.setBrightness(0x00); 
  display.showNumberDec(n, false, 4, 0);
}

void displayOff()
{
  display.setBrightness(0x00,false);  
}

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);
  digitalWrite(BUILTIN_LED, !state);
}

void doDelays() {
  long endMs = millis() + 1000;
  while (millis() < endMs) {
    yield();
  }
}
void setup() {
  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  ticker.attach(0.5, tick);
  displayOff();
  
  USE_SERIAL.begin(115200);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  USE_SERIAL.println("Autoconnect...");
  wifiManager.setAPCallback(configModeCallback);
  WiFiManagerParameter custom_text("<p>Solar Power Meter by Paul</p>");
  wifiManager.addParameter(&custom_text);
  if (!wifiManager.autoConnect("Solar-Meter", "password")) {
    USE_SERIAL.println("failed to connect and hit timeout. Resetting...");
    ESP.reset();
    delay(1000);
  }

  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);

  for (uint8_t t = 4; t > 0; t--) {
    showNum(t);
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  USE_SERIAL.println(WiFi.localIP());
  fullUrl = findIp();
}

void configModeCallback (WiFiManager *myWiFiManager) {
  USE_SERIAL.println("Entered config mode");
  USE_SERIAL.println(WiFi.softAPIP());
  USE_SERIAL.println(myWiFiManager->getConfigPortalSSID());
  ticker.attach(0.2, tick);
}

int parseJson(String payload) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(payload);
  int watts = root["Body"]["Data"]["PAC"]["Values"]["1"];
  return watts;
}

String findIp() {
  String network = String(WiFi.localIP()[0])+"."+String(WiFi.localIP()[1])+"."+String(WiFi.localIP()[2])+".";
  USE_SERIAL.println("Scanning: " + network);

  HTTPClient http;
  http.setTimeout(500);
  
  for (int x=0;x<5;x++) {
    for (int i=1;i<255;i++) {
      String ip = network + String(i);
      String fullUrl = "http://" + ip + url;
      USE_SERIAL.println("Checking: " + fullUrl);
      http.begin(fullUrl);
      int httpCode = http.GET();
      if (httpCode == 200) {
        USE_SERIAL.println("200 Success");
        http.end();
        return fullUrl;
      } else {
        USE_SERIAL.println(String(httpCode) + " Fail");
      }
      http.end();
    }
  }
}

void loop() {
  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    USE_SERIAL.print("[HTTP] begin...\n");
    http.begin(fullUrl); //HTTP

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
      int httpCode = http.GET();
      
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        int watts = parseJson(payload);
        showNum(watts);
        USE_SERIAL.println(String(watts));
      }
      retryCount =0;
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());

      if (retryCount < 1)
      {
        showNumDim(0);   
      }
      else if (retryCount < 10)
      {
        displayOff();
      }
      retryCount += 1;
    }
    http.end();
  
  }
   doDelays();
}
