#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <HTTPClient.h>

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
const char *ssid = "";      // replace with you ssid
const char *password = "";  // replace with your password
const char *GScriptId = ""; // replace with your script id
const char *batteryHost = NULL;
#endif

#define uS_TO_S_FACTOR 1000000ULL   // Conversion factor from microseconds to seconds
#define TIME_TO_SLEEP  1800             // Time for ESP32-E to enter deep sleep

const char *weather_cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\n"
    "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\n"
    "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\n"
    "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\n"
    "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\n"
    "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\n"
    "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\n"
    "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\n"
    "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\n"
    "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\n"
    "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\n"
    "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\n"
    "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\n"
    "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\n"
    "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\n"
    "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\n"
    "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\n"
    "rqXRfboQnoZsG4q5WTP468SQvvG5\n"
    "-----END CERTIFICATE-----\n";

const char *events_cert =
    "-----BEGIN CERTIFICATE-----\n"
    "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\n"
    "bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c\n"
    "-----END CERTIFICATE-----\n";

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

WeatherInfo getWeatherInfo(String station, const char *host, String path, const char *cert, int &error)
{
  Serial.println("Getting weather info");
  error = 0;
  WiFiClientSecure httpsClient;

  httpsClient.setCACert(cert);
  int retry = 0;
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
    return {0, 0, 0};
  }
  else
  {
    DPRINTLN("Connected to Server");
  }
  httpsClient.print(String("GET ") + path + " HTTP/1.1\r\n" +
                    "Host: " + host + "\r\n" +
                    "User-Agent: BuildFailureDetectorESP8266\r\n" +
                    "Connection: close\r\n\r\n");
  delay(100);
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
    if (line.startsWith(station))
    {
      String values = line.substring(line.indexOf(';') + 1);
      values = values.substring(values.indexOf(';') + 1);
      double temp = values.substring(0, values.indexOf(';')).toDouble();
      values = values.substring(values.indexOf(';') + 1);
      double rain = values.substring(0, values.indexOf(';')).toDouble();
      values = values.substring(values.indexOf(';') + 1);
      double sunshine = values.substring(0, values.indexOf(';')).toDouble();
      return {temp, sunshine, rain};
    }
  }
  error = 2;
  return {0, 0, 0};
}

void getCalendarInfo(char *events, const char *cert, int &error)
{
  error = 0;
  Serial.println("Getting calendar info");

  HTTPClient http;
  String url = String("https://script.google.com/macros/s/") + GScriptId + "/exec?read";
  http.begin(url.c_str()); // Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  String payload;
  if (httpCode > 0)
  { // Check for the returning code
    payload = http.getString();
    StaticJsonDocument<JSON_ARRAY_SIZE(10) + 200> doc;

    DeserializationError jsonError = deserializeJson(doc, payload);
    if (jsonError)
    {
      Serial.println(payload);
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
}

void setup()
{
  WiFi.mode(WIFI_STA);
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

  // sendBatteryState(ESP.getVcc());
  int weatherError = 0;

  WeatherInfo weatherInfo = getWeatherInfo("SMA", "data.geo.admin.ch", "/ch.meteoschweiz.messwerte-aktuell/VQHA80.csv", weather_cert, weatherError);
  // retry if station was not found
  if (weatherError == 2)
  {
    weatherError = 0;
    weatherInfo = getWeatherInfo("SMA", "data.geo.admin.ch", "/ch.meteoschweiz.messwerte-aktuell/VQHA80.csv", weather_cert, weatherError);
  }

  char events[300];
  int eventError = 0;
  getCalendarInfo(events, events_cert, eventError);

  screen.clear();
  screen.printCalendarData(events, eventError);
  screen.printWeatherData(weatherInfo, weatherError);
  screen.update();
  screen.powerOff();

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR); 
  esp_deep_sleep_start();
}

void loop()
{
  // put your main code here, to run repeatedly:
}