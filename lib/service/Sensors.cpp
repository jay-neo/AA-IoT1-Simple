#include "Sensors.hpp"

#include <ArduinoJson.h>
#include <DHT.h>

// Public Constructor
Sensors::Sensors() : dht(nullptr) {}

// Public Destructor
Sensors::~Sensors() {
    if(dht != nullptr) {
        delete dht;
    }
}

// Public Setter
void Sensors::set_ph_pin(const uint8_t _pin) {
    this->ph_pin = _pin;
    pinMode(this->ph_pin, INPUT);
}
void Sensors::set_moisture_pin(const uint8_t _pin) { this->moisture_pin = _pin; }
void Sensors::set_dht(const uint8_t _pin, const uint8_t _type) {
    if(dht != nullptr) {
        delete dht;
    }
    this->dht_pin = _pin;
    this->dht_type = _type;
    this->dht = new DHT(this->dht_pin, this->dht_type);
    this->dht->begin();
}
void Sensors::set_npk(const uint8_t _RE, const uint8_t _DE, const uint8_t _RX, const uint8_t _TX,
                      const uint8_t code[], const uint32_t speed) {
    this->RE_npk_pin = _RE;
    this->DE_npk_pin = _DE;
    this->npk_sensor = new SoftwareSerial(_RX, _TX);
    this->npk_sensor->begin(speed);
    for(size_t i = 0; i < 8; ++i) {
        this->npk_sensor_code[i] = code[i];
    }
    pinMode(_RE, OUTPUT);
    pinMode(_DE, OUTPUT);
    delay(2000);
}

// Private Getter
float Sensors::get_humidity() {}
float Sensors::get_temperature() {}
float Sensors::get_ph() {
    float rawpH = analogRead(this->ph_pin);
    float actualpH = ((0.795 * (rawpH * 3.30 / 4095)) - 1.63);
    return actualpH;
}
float Sensors::get_moisture() {
    const int valAir = 1550;
    const int valWater = 1065;

    int soilMoistureValue = analogRead(this->moisture_pin);
    if(isnan(soilMoistureValue)) {
        Serial.println("Failed to read from DHT Sensor: Humidity");
    }

    float soilMoisture = map(soilMoistureValue, valAir, valWater, 0, 100);
    soilMoisture = constrain(soilMoisture, 0, 100);

    return soilMoisture;
}
void Sensors::read_dht(float &humidity, float &temperature) {
    humidity = this->dht->readHumidity();
    temperature = this->dht->readTemperature();
    if(isnan(humidity)) {
        Serial.println("Failed to read from DHT Sensor: Humidity");
    }
    if(isnan(temperature)) {
        Serial.println("Failed to read from DHT Sensor: Temperature");
    }
}
void Sensors::read_npk(float &N, float &P, float &K) {
    digitalWrite(this->DE_npk_pin, HIGH);
    digitalWrite(this->RE_npk_pin, HIGH);
    uint8_t values[11];

    if(this->npk_sensor->write(this->npk_sensor_code, sizeof(this->npk_sensor_code)) == 8) {
        digitalWrite(this->DE_npk_pin, LOW);
        digitalWrite(this->RE_npk_pin, LOW);
        delay(100);

        // Check if we have enough data to read (11 bytes expected)
        if(this->npk_sensor->available() >= 11) {
            for(int i = 0; i < 11; i++) {
                values[i] = this->npk_sensor->read();
            }

            // Combine two bytes for each value (16-bit data)
            N = (values[3] << 8) | values[4];
            P = (values[5] << 8) | values[6];
            K = (values[7] << 8) | values[8];

        } else {
            Serial.println("No data received");
        }
    }
}

// Public API
void Sensors::read_all() {
    // pH
    if(this->ph_pin != 0) {
        res["ph"] = this->get_ph();
    }
    // Soil Moisture
    if(this->moisture_pin != 0) {
        this->res["moisture"] = this->get_moisture();
    }
    // DHT
    if(this->dht_pin != 0) {
        float humidity = 0, temperature = 0;
        this->read_dht(humidity, temperature);
        this->res["humidity"] = humidity, res["temperature"] = temperature;
    }
    // NPK
    if(this->RE_npk_pin != 0 && this->DE_npk_pin != 0) {
        float N = 0, P = 0, K = 0;
        this->read_npk(N, P, K);
        res["nitrogen"] = N, res["phosphorus"] = P, res["potassium"] = K;
    }
}

// Protected Getter
DynamicJsonDocument Sensors::get_all() {
    DynamicJsonDocument doc(JSON_OBJECT_SIZE(res.size() + 2));
    for(const auto &kv : res) {
        doc[kv.first.c_str()] = kv.second;
    }
    return doc;
}