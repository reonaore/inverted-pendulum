#ifndef _WIFI_CONFIG_HPP_
#define _WIFI_CONFIG_HPP_

#include <ArduinoJson.h>
#include <M5StickCPlus2.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "hardware.hpp"

class wifiConfig {
 private:
  /* data */
  static wifiConfig fromJsonFile(File stream) {
    JsonDocument doc;
    deserializeJson(doc, stream);
    return wifiConfig(doc["ssid"], doc["password"]);
  }
  wifiConfig(const char* ssid, const char* password)
      : ssid(ssid), password(password) {};

 public:
  String ssid;
  String password;
  static wifiConfig fromFile(const char* filePath = "/wifi-config.json") {
    if (!SPIFFS.exists(filePath)) {
      throw "file not exist";
    }

    auto f = SPIFFS.open(filePath);
    try {
      auto res = wifiConfig::fromJsonFile(f);
      f.close();
      return res;
    } catch (const char* e) {
      f.close();
      throw e;
    }
  }
  void begin() {
    WiFi.mode(WIFI_STA);
    WiFi.hostname(HOST_NAME);
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print("connecting...");
      delay(500);
    }
  }
  wifiConfig() {};
  ~wifiConfig() {};
};

#endif
