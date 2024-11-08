#include "AgriArenaClient.hpp"

bool AgriArenaClient::config(const String &URL, const char *certificate) {
#ifdef USE_HTTPS
    client.setCACert(certificate);
#endif
    client.setTimeout(10000);
    http.begin(client, URL);
    http.addHeader("Content-Type", "application/json");

    deviceId = ESP.getEfuseMac();
    return true;
}

void AgriArenaClient::send_all(DynamicJsonDocument data) {
    data["iot"] = deviceId;

    String jsonString;
    serializeJson(data, jsonString);

    Serial.println(jsonString);

    int httpResponseCode = http.POST(jsonString);
    if(httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    } else {
        Serial.print("ERROR: Sending data to server ");
        Serial.println(httpResponseCode);
    }
}
