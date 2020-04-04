#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <esp8266httpclient.h>

#include "HTTPSRedirect.h"
#include "screen.h"
#include "credentials.h"
#include "DebugMacros.h"

#ifdef CREDENTIALS
const char *ssid = mySSID;
const char *password = myPASSWORD;
const char *GScriptId = scriptID;
const char *batteryHost = myBatteryHost;
int batteryPort = myBatteryPort;
const char *batteryUrl = myBatteryUrl;
#else
const char *ssid = "";      //replace with you ssid
const char *password = "";  //replace with your password
const char *GScriptId = ""; //replace with your script id
const char *batteryHost = NULL;

#endif

ADC_MODE(ADC_VCC);
EinkScreen screen = EinkScreen();
void sendBatteryState(uint16_t value)
{
  if (batteryHost == NULL)
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

WeatherInfo getWeatherInfo(String station, const char *host, String path, const char fingerprint[], int &error)
{
  error = 0;
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
    error = 1;
    return {0,0,0};
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
  while (httpsClient.available())
  {
    String line = httpsClient.readStringUntil('\n');
    if(line.startsWith(station))
    {
      String values = line.substring(line.indexOf(';')+1);
      values = values.substring(values.indexOf(';')+1);
      double temp = values.substring(0,values.indexOf(';')).toDouble();
      values = values.substring(values.indexOf(';')+1);
      double rain =values.substring(0,values.indexOf(';')).toDouble();
      values = values.substring(values.indexOf(';')+1);
      double sunshine = values.substring(0,values.indexOf(';')).toDouble();
      return {temp, sunshine, rain};
    }
  }
  error = 2;
  return {0,0,0};
}

void getCalendarInfo(char *events, int &error)
{
  error = 0;
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
  DeserializationError jsonError = deserializeJson(doc, data);
  if (jsonError)
  {
    Serial.println(data);
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(jsonError.c_str());
    error = 1;
  }
  if (!doc.is<JsonArray>())
  {
    Serial.println("Response is not a JSON Array");
    error = 2;
  }
  DPRINT("# Events ");
  DPRINTLN(doc.size());
  strcpy(events, "");
  for (u_int i = 0; i < doc.size(); i++)
  {
    const char *event = doc[i];
    DPRINTLN(event);
    strcat(events, event);
    strcat(events, "\n");
  }
  DPRINTLN("Events:");
  DPRINTLN(events);
}

void setup()
{
  pinMode(D6, OUTPUT);
  digitalWrite(D6, LOW);
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
  int weatherError = 0;
  WeatherInfo weatherInfo = getWeatherInfo("SMA","data.geo.admin.ch","/ch.meteoschweiz.messwerte-aktuell/VQHA80.csv","f635f3f089482deac772461781834c13291d62ba",weatherError);

  char events[300];
  int eventError = 0;
  getCalendarInfo(events, eventError);

  screen.clear();
  if (eventError == 0)
  {
    screen.printCalendarData(events);
  }
  if (weatherError == 0)
  {
    screen.printWeatherData(weatherInfo);
  }
  screen.update();
  screen.powerOff();
  digitalWrite(D6, HIGH);
}

void loop()
{
  // put your main code here, to run repeatedly:
}