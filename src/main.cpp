#include "config.hpp"

#if defined(DISPLAY_WIDTH) && defined(DISPLAY_HEIGHT)
Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HEIGHT);
#endif

#if defined(DHT_PIN)
DHT dht(DHT_PIN, DHT_TYPE);
#endif

#if defined(NPK_RX) && defined(NPK_TX)
SoftwareSerial npk(NPK_RX, NPK_TX);
#endif

WifiConfiguration wifi(WIFI_SSID, WIFI_PASSWORD);
AgriArenaClient client;

float A = 0, B = 0, C = 0;

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void read_dht() {
    uint8_t r = 0;
    A = dht.readHumidity();
    while(isnan(A) && r < 10) {
        A = dht.readHumidity();
        ++r;
        delay(500);
    }
    if(r >= 10) {
        Serial.println("ERROR: Humidity Reading");
    }

    r = 0;
    B = dht.readTemperature();
    while(isnan(B) && r < 10) {
        dht.readTemperature();
        ++r;
        delay(500);
    }
    if(r >= 10) {
        Serial.println("ERROR: Temperature Reading");
    }
}
float read_moisture() {
    int soilMoistureValue = analogRead(MOISTURE_PIN);
    uint8_t r = 0;
    while(isnan(soilMoistureValue) && r < 10) {
        soilMoistureValue = analogRead(MOISTURE_PIN);
        ++r;
        delay(500);
    }

    if(r >= 10) {
        Serial.println("ERROR: Moisture Reading");
    }

    float soilMoisture = map(soilMoistureValue, MOISTURE_LOWER_LIMIT, MOISTURE_UPPER_LIMIT, 0, 100);
    return constrain(soilMoisture, 0, 100);
}
float read_ph() {
    // float rawpH = analogRead(PH_PIN);
    // return ((0.795 * (rawpH * 3.30 / 4095)) - 1.63);

    float raw_ph = 0;
    for(int i = 0; i < 6; i++) {
        raw_ph += analogRead(PH_PIN);
        delay(1000);
    }
    if(isnan(raw_ph)) {
        Serial.println("ERROR: PH Reading");
    }

    float cal_ph = ((float)raw_ph * 3.30 / 4095);
    float res_ph = ((0.795 * cal_ph) - 1.63);
    return res_ph;
}
void read_npk() {
    digitalWrite(NPK_RE, HIGH);
    digitalWrite(NPK_DE, HIGH);
    uint8_t values[11];
    uint8_t r = 0;
    uint8_t s = 0;

    while(r < 5 && s == 1 && (npk.write(NPK_CODE, sizeof(NPK_CODE)) == 8)) {
        digitalWrite(NPK_RE, LOW);
        digitalWrite(NPK_DE, LOW);
        delay(500);
        if(npk.available() >= 11) {
            for(int i = 0; i < 11; i++) {
                values[i] = npk.read();
            }
            A = (values[3] << 8) | values[4];
            B = (values[5] << 8) | values[6];
            C = (values[7] << 8) | values[8];
            s = 1;

        } else {
            r++;
        }
    }
    if((s == 0 && r >= 5) || (isnan(A) && isnan(B) && isnan(C))) {
        Serial.println("ERROR: NPK Reading");
    }
}

void agri_arena_iot() {
    std::map<String, float> res;

#if defined(MOISTURE_PIN)
    res["moisture"] = read_moisture();
    delay(1000);
#endif

#if defined(DHT_PIN)
    A = B = 0;
    read_dht();
    res["humidity"] = A, res["temperature"] = B;
    delay(1000);
#endif

#if defined(PH_PIN)
    res["ph"] = read_ph();
    delay(1000);
#endif

#if defined(NPK_RE) && defined(NPK_DE)
    A = B = C = 0;
    read_npk();
    res["nitrogen"] = A, res["phosphorus"] = B, res["potassium"] = C;
#endif

    DynamicJsonDocument data(JSON_OBJECT_SIZE(res.size() + 2));
    for(const auto& kv : res) {
        data[kv.first.c_str()] = kv.second;
    }

    client.send_all(data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    Serial.println("Starting...");

    if(!client.config(ENDPOINT, TLS_CERTIFICATE)) {
        Serial.println("ERROR: Server not connected successfully");
        // esp_deep_sleep_start();
    }

    if(!wifi.config()) {
        Serial.println("ERROR: Wifi not configured successfully");
        // esp_deep_sleep_start();
    }

#if defined(DHT_PIN)
    dht.begin();
    delay(2000);
    for(int8_t i = 0; i < 5; ++i) {
        read_dht();
    }
#endif

#if defined(MOISTURE_PIN)
    pinMode(MOISTURE_PIN, INPUT);
    delay(2000);
    for(int8_t i = 0; i < 5; ++i) {
        read_moisture();
    }
#endif

#if defined(PH_PIN)
    for(int8_t i = 0; i < 5; ++i) {
        read_ph();
    }
    delay(2000);
#endif

#if defined(NPK_RE) && defined(NPK_DE)
    npk.begin(NPK_BAUD_RATE);
    pinMode(NPK_RE, OUTPUT);
    pinMode(NPK_DE, OUTPUT);
    delay(2000);
    for(int8_t i = 0; i < 5; ++i) {
        read_npk();
    }
#endif

#if defined(DISPLAY_WIDTH) && defined(DISPLAY_HEIGHT)
    if(!display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
    } else {
        display.display();
        delay(2000);
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);

        display.setTextSize(1);
        display.setCursor(10, 10);
        display.println("Wellcome to AgriArena");
        delay(2000);

        // display.clearDisplay();
        // display.setTextSize(0);
        // display.setCursor(0, 0);
    }
#endif

    delay(5000);
}

void loop() {
    if(wifi.isAlive()) {
        agri_arena_iot();
#if defined(DISPLAY_WIDTH) && defined(DISPLAY_HEIGHT)
        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(5, 5);
        display.println("AGRI ARENA");
#endif
    } else {
        Serial.println("ERROR: Wifi disconnectd");
        if(!wifi.connect()) {
            Serial.println("ERROR: Failed to connect to WiFi");
        }
    }
    delay(3000);
}