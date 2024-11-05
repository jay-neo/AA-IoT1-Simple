#ifdef USE_HTTPS
#include <WiFiClientSecure.h>
#else
#include <WiFi.h>
#endif

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#include <map>

#include "WifiConfiguration.hpp"

// ---------------------------------------------------------------------------
// For General Purpose
#define SWITCH 0

// For OLED Display
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define DISPLAY_ADDRESS 0x3C

// For DHT Sensor
#define DHT_PIN 4
#define DHT_TYPE DHT22

// For Moisture Sensor
#define MOISTURE_PIN 12

// For PH Sensor
#define PH_PIN 34

// For NPK Sensor
#define NPK_RE 5
#define NPK_DE 2
#define NPK_RX 22
#define NPK_TX 23
const uint8_t NPK_CODE[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x03, 0x65, 0xCD};
const uint32_t NPK_BAUD_RATE = 9600;

// For Server Endpoint
const String ENDPOINT = "http://localhost:3000/api/iot";

// For ESP32 Internet Connection
const String WIFI_SSID = "BABU";
const String WIFI_PASSWORD = "BABUMANU@02";
const char *TLS_CERTIFICATE = R"KEY(

)KEY";