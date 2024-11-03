#ifdef USE_HTTPS
#include <WiFiClientSecure.h>
#else
#include <WiFi.h>
#endif

#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>

#include <map>

#include "OLED.hpp"
#include "WifiConfiguration.hpp"

// ---------------------------------------------------------------------------

// For General Purpose
const uint8_t SWITCH = 0;

// For OLED Display
const uint8_t DISPLAY_WIDTH = 0;
const uint8_t DISPLAY_HEIGHT = 0;
const uint8_t DISPLAY_ADDRESS = 0;

// For DHT Sensor
const uint8_t DHT_PIN = 4;
const uint8_t DHT_TYPE = DHT22;

// For Moisture Sensor
const uint8_t MOISTURE_PIN = 12;

// For PH Sensor
const uint8_t PH_PIN = 0;

// For NPK Sensor
const uint8_t NPK_RE = 0;
const uint8_t NPK_DE = 0;
const uint8_t NPK_RX = 0;
const uint8_t NPK_TX = 0;
const uint8_t NPK_CODE[] = {};
const uint32_t NPK_BAUD_RATE = 0;

// For Server Endpoint
const String ENDPOINT = "http://localhost:3000/api/iot";

// For ESP32 Internet Connection
const String WIFI_SSID = "";
const String WIFI_PASSWORD = "";
const char *TLS_CERTIFICATE = R"KEY(

)KEY";
