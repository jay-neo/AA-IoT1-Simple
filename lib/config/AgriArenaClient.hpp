#ifndef AGRIARENACLIENT_HPP
#define AGRIARENACLIENT_HPP

#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "OLED.hpp"
#include "Sensors.hpp"

#ifdef USE_HTTPS
#include <WiFiClientSecure.h>
#else
#include <WiFi.h>
#endif

class AgriArenaClient {
#ifdef USE_HTTPS
    WiFiClientSecure client;
#else
    WiFiClient client;
#endif

    HTTPClient http;
    uint64_t deviceId;

   public:
    void send_all(DynamicJsonDocument data);
    bool config(const String &URL, const char *certificate);
};

#endif