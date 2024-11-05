#include "config.hpp"

#if defined(DISPLAY_WIDTH) && defined(DISPLAY_HEIGHT)
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
#endif

#if defined(DHT_PIN) && (DHT_PIN != 0)
DHT dht(DHT_PIN, DHT_TYPE);
#endif

#if defined(NPK_RX) && defined(NPK_TX)
SoftwareSerial npk(NPK_RX, NPK_TX);
#endif

WifiConfiguration wifi(WIFI_SSID, WIFI_PASSWORD);
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

#ifdef USE_HTTPS
    client.setCACert(certificate);
#endif
    client.setTimeout(10000);
    http.begin(client, ENDPOINT);
    http.addHeader("Content-Type", "application/json");

    if(!wifi.config()) {
        Serial.println("Wifi not configured successfully!");
        // esp_deep_sleep_start();
    }

#if defined(DHT_PIN)
    dht.begin();
    delay(1000);
#endif

#if defined(MOISTURE_PIN)
    pinMode(MOISTURE_PIN, INPUT);
    delay(1000);
#endif

#if defined(NPK_RE) && defined(NPK_DE)
    npk.begin(NPK_BAUD_RATE);
    pinMode(NPK_RE, OUTPUT);
    pinMode(NPK_DE, OUTPUT);
    delay(1000);
#endif

#if defined(DISPLAY_WIDTH) && defined(DISPLAY_HEIGHT)
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
#endif

    delay(5000);
}

void agri_arena_iot() {
    std::map<String, float> res;

// pH
#if defined(PH_PIN)
    float rawpH = analogRead(PH_PIN);
    res["ph"] = ((0.795 * (rawpH * 3.30 / 4095)) - 1.63);
    delay(1000);
#endif

// Soil Moisture
#if defined(MOISTURE_PIN)
    const int valAir = 1550;
    const int valWater = 1065;

    int soilMoistureValue = analogRead(MOISTURE_PIN);
    if(isnan(soilMoistureValue)) {
        Serial.println("Failed to read from DHT Sensor: Humidity");
    }

    float soilMoisture = map(soilMoistureValue, valAir, valWater, 0, 100);
    res["moisture"] = constrain(soilMoisture, 0, 100);
    delay(1000);
#endif

// DHT
#if defined(DHT_PIN)
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
    delay(1000);
#endif

// NPK
#if defined(NPK_RE) && defined(NPK_DE)
    float N = 0, P = 0, K = 0;
    digitalWrite(NPK_RE, HIGH);
    digitalWrite(NPK_DE, HIGH);
    uint8_t values[11];

    if(npk.write(NPK_CODE, sizeof(NPK_CODE)) == 8) {
        digitalWrite(NPK_RE, LOW);
        digitalWrite(NPK_DE, LOW);
        delay(1000);

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
    delay(1000);
#endif

    DynamicJsonDocument data(JSON_OBJECT_SIZE(res.size() + 2));
    for(const auto &kv : res) {
        data[kv.first.c_str()] = kv.second;
    }

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
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
    }
    http.end();
}

void loop() {
    if(wifi.isAlive()) {
        agri_arena_iot();
    } else {
        Serial.println("Wifi disconnectd!");
    }
    delay(3000);
}