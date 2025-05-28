#include <Arduino.h>
#include <WiFi.h>
#include "WebSocket.h"

const char *SSID = "HUAWEI";
const char *PASSWORD = "88888888";
void initWiFi();

void setup()
{
  Serial.begin(115200);
  delay(500);
  initWiFi();
  static httpd_handle_t server = null;
  server = start_webserver();
  
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
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