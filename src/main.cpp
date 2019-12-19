#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <esp8266httpclient.h>

#include "HTTPSRedirect.h"
#include "screen.h"
#include "credentials.h"
#include "DebugMacros.h"

#ifdef CREDENTIALS
const char* ssid = mySSID;
const char* password = myPASSWORD;
const char* GScriptId = scriptID;
const char* batteryHost = myBatteryHost;
int batteryPort = myBatteryPort;
const char* batteryUrl = myBatteryUrl;
#else
const char* ssid = ""; //replace with you ssid
const char* password = ""; //replace with your password
const char* GScriptId = ""; //replace with your script id
const char* batteryHost = NULL;

#endif

ADC_MODE(ADC_VCC);
EinkScreen screen = EinkScreen();
void sendBatteryState(uint16_t value)
{
  if(batteryHost==NULL)
  {
    return;
  }
  WiFiClient httpClient;
  int retry = 0;
  while ((!httpClient.connect(batteryHost, batteryPort)) && (retry < 15))
  {
    delay(100);
    DPRINT(".");
    retry++;
  }
  if (retry == 15)
  {
    DPRINTLN("Connection failed");
  }
  else
  {
    DPRINTLN("Connected to Server");
  }

  httpClient.print(String("PUT ") + batteryUrl + " HTTP/1.1\r\n" +
                    "Host: " + batteryHost + "\r\n" +
                    "User-Agent: BuildFailureDetectorESP8266\r\n" +
                    "Content-Type: text/plain\r\n" +
                    "Content-Length: 4\r\n" +
                    "Connection: close\r\n\r\n" +
                    value);
  DPRINT("Battery value:");
  DPRINTLN(value);
  while (httpClient.connected())
  {
    String line = httpClient.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
  }
  String result;
  while (httpClient.available())
  {
    result += httpClient.readStringUntil('\n');
  }
  DPRINTLN(result);
}

JsonDocument jsonGetRequest(const char *host, String path, const char fingerprint[], const size_t capacity)
{
  WiFiClientSecure httpsClient;
  int retry = 0;
  httpsClient.setFingerprint(fingerprint);
  while ((!httpsClient.connect(host, 443)) && (retry < 15))
  {
    delay(100);
    Serial.print(".");
    retry++;
  }
  if (retry == 15)
  {
    Serial.println("Connection failed");
  }
  else
  {
    DPRINTLN("Connected to Server");
  }

  httpsClient.print(String("GET ") + path + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "User-Agent: BuildFailureDetectorESP8266\r\n" +
                    "Connection: close\r\n\r\n");
  while (httpsClient.connected())
  {
    String line = httpsClient.readStringUntil('\n');
    if (line == "\r")
    {
      break;
    }
  }
  String result;
  while (httpsClient.available())
  {
    result += httpsClient.readStringUntil('\n');
  }
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, result);
  if (error)
  {
    Serial.println(result);
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }
  return doc;
}

void getCalendarInfo(char* events)
{
  const char *host = "script.google.com";
  String url = String("/macros/s/") + GScriptId + "/exec";

  Serial.println("Getting calendar info");
  StaticJsonDocument<JSON_ARRAY_SIZE(10) + 200> doc;

  const int httpsPort = 443;
  HTTPSRedirect client = HTTPSRedirect(httpsPort);
  client.setInsecure();
  //client.setFingerprint("96383360d46b84c932674944f227d87c331a355a");
  client.setPrintResponseBody(true);
  if (!client.connected())
    client.connect(host, httpsPort);

  client.GET(url, host);
  String data = client.getResponseBody();
  DeserializationError error = deserializeJson(doc, data);
  if (error)
  {
    Serial.println(data);
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
  }
  if (!doc.is<JsonArray>())
  {
    Serial.println("Response is not a JSON Array");
  }
  DPRINT("# Events ");
  DPRINTLN(doc.size());
  strcpy(events, "");
  for (u_int i = 0;i < doc.size(); i++)
  {
    const char* event = doc[i];
    DPRINTLN(event);
    strcat(events,event);
    strcat(events,"\n");
  }
  DPRINTLN("Events:");
  DPRINTLN(events);
}

WeatherInfo getWeatherInfo()
{
  Serial.println("Getting weather info");
  const size_t capacity = JSON_OBJECT_SIZE(7) + JSON_OBJECT_SIZE(13) + 310;
  JsonDocument doc = jsonGetRequest("opendata.netcetera.com", "/smn/smn/SMA", "63e90ab49c379653aed3d0c5b5e10bddbd9a05f3", capacity);
  return {doc["temperature"], doc["sunshine"], doc["precipitation"]};
}

void setup()
{
  pinMode(D6, OUTPUT);
  digitalWrite(D6,LOW);
  WiFi.begin(ssid, password);
  Serial.begin(115200);
  Serial.println();
  Serial.println("Connecting to WIFI");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(100);

  sendBatteryState(ESP.getVcc());

  // first update should be full refresh
  screen.clear();
  screen.printWeatherData(getWeatherInfo());
  char events[300];
  getCalendarInfo(events);
  screen.printCalendarData(events);
  screen.update();
  screen.powerOff();
  digitalWrite(D6,HIGH);
}

void loop()
{
  // put your main code here, to run repeatedly:
}