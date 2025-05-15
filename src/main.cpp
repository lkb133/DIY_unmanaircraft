#include <Arduino.h>
#include <WiFi.h>

const char *SSID = "HUAWEI";
const char *PASSWORD = "88888888";
void initWiFi();

void setup()
{
  Serial.begin(115200);
  delay(500);
  initWiFi();
}

void loop()
{
}

// function
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("try to connect [%s]", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.printf("%s\n", WiFi.localIP().toString().c_str());
}