#ifndef SENSORS_HPP
#define SENSORS_HPP

#include <ArduinoJson.h>
#include <DHT.h>
#include <SoftwareSerial.h>

#include <map>
#include <vector>

class Sensors {
   private:
    uint8_t dht_pin;
    uint8_t dht_type;
    uint8_t moisture_pin;
    uint8_t ph_pin;

    uint8_t RE_npk_pin;
    uint8_t DE_npk_pin;
    uint8_t npk_sensor_code[8];
    SoftwareSerial *npk_sensor;

    DHT *dht;
    std::map<String, float> res;

    // Private Getter
    float get_ph();
    float get_moisture();
    float get_humidity();
    float get_temperature();
    void read_dht(float &humidity, float &temperature);
    void read_npk(float &N, float &P, float &K);

   protected:
    // Protected Getter

   public:
    Sensors();
    ~Sensors();
    DynamicJsonDocument get_all();

    // Public Setter
    void set_ph_pin(const uint8_t _pin);
    void set_moisture_pin(const uint8_t _pin);
    void set_dht(const uint8_t _pin, const uint8_t _type);
    void set_npk(const uint8_t _RE, const uint8_t _DE, const uint8_t _RX, const uint8_t _TX, const uint8_t[],
                 const uint32_t speed);

    // Public Utilities
    void read_all();
};

#endif