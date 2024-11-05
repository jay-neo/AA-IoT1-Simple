#include "config.hpp"

WifiConfiguration wifi(WIFI_SSID, WIFI_PASSWORD);

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT);

DHT dht(DHT_PIN, DHT_TYPE);
SoftwareSerial npk(NPK_RX, NPK_TX);

const uint64_t deviceId = ESP.getEfuseMac();

#ifdef USE_HTTPS
WiFiClientSecure client;
#else
WiFiClient client;
#endif

HTTPClient http;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    npk.begin(NPK_BAUD_RATE);
    pinMode(NPK_RE, OUTPUT);
    pinMode(NPK_DE, OUTPUT);
    delay(2000);

#ifdef USE_HTTPS
    this->client.setCACert(certificate);
#endif
    http.begin(client, ENDPOINT);
    http.addHeader("Content-Type", "application/json");

    if(!wifi.config()) {
        Serial.println("Wifi not configured successfully!");
        esp_deep_sleep_start();
    }

    if(display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        esp_deep_sleep_start();
    }
    display.display();
    delay(2000);

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);
    display.setCursor(10, 10);
    display.println("AGRI ARENA");
    delay(2000);

    display.clearDisplay();
    display.setTextSize(0);
    display.setCursor(0, 0);
}

void agri_arena_iot1() {
    std::map<String, float> res;
    // pH
    if(PH_PIN != 0) {
        float rawpH = analogRead(PH_PIN);
        res["ph"] = ((0.795 * (rawpH * 3.30 / 4095)) - 1.63);
    }
    // Soil Moisture
    if(MOISTURE_PIN != 0) {
        const int valAir = 1550;
        const int valWater = 1065;

        int soilMoistureValue = analogRead(MOISTURE_PIN);
        if(isnan(soilMoistureValue)) {
            Serial.println("Failed to read from DHT Sensor: Humidity");
        }

        float soilMoisture = map(soilMoistureValue, valAir, valWater, 0, 100);
        res["moisture"] = constrain(soilMoisture, 0, 100);
    }
    // DHT
    if(DHT_PIN != 0) {
        float H = dht.readHumidity();
        float T = dht.readTemperature();
        if(isnan(H)) {
            Serial.println("Failed to read from DHT Sensor: Humidity");
        }
        if(isnan(T)) {
            Serial.println("Failed to read from DHT Sensor: Temperature");
        }
        res["humidity"] = H;
        res["temperature"] = T;
    }
    // NPK
    if(NPK_RE != 0 && NPK_DE != 0) {
        float N = 0, P = 0, K = 0;
        digitalWrite(NPK_RE, HIGH);
        digitalWrite(NPK_DE, HIGH);
        uint8_t values[11];

        if(npk.write(NPK_CODE, sizeof(NPK_CODE)) == 8) {
            digitalWrite(NPK_RE, LOW);
            digitalWrite(NPK_DE, LOW);
            delay(100);

            // Check if we have enough data to read (11 bytes expected)
            if(npk.available() >= 11) {
                for(int i = 0; i < 11; i++) {
                    values[i] = npk.read();
                }

                // Combine two bytes for each value (16-bit data)
                N = (values[3] << 8) | values[4];
                P = (values[5] << 8) | values[6];
                K = (values[7] << 8) | values[8];

            } else {
                Serial.println("No data received");
            }
        }

        res["nitrogen"] = N, res["phosphorus"] = P, res["potassium"] = K;
    }

    // After collecting all experiments send these to the server
    DynamicJsonDocument data(JSON_OBJECT_SIZE(res.size() + 2));
    for(const auto &kv : res) {
        data[kv.first.c_str()] = kv.second;
    }

    data["iot"] = deviceId;

    String jsonString;
    serializeJson(data, jsonString);

    int httpResponseCode = http.POST(jsonString);
    if(httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
    } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
}

void loop() {
    if(wifi.isAlive()) {
        agri_arena_iot1();
    } else {
        Serial.println("Wifi disconnectd!");
    }
    delay(5000);
}