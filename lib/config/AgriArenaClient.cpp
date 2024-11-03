#include <ArduinoJson.h>

#include "AgriArenaClient.hpp"

bool AgriArenaClient::config(const String &URL, const char *certificate) {
#ifdef USE_HTTPS
    this->client.setCACert(certificate);
#endif
    this->http.begin(client, URL);
    this->http.addHeader("Content-Type", "application/json");

    this->deviceId = ESP.getEfuseMac();

    return true;
}

void AgriArenaClient::send_all(DynamicJsonDocument data) {
    data["iot"] = this->deviceId;

    String jsonString;
    serializeJson(data, jsonString);

    int httpResponseCode = this->http.POST(jsonString);
    if(httpResponseCode > 0) {
        String response = this->http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
}
