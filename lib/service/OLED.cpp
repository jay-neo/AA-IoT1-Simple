#include "OLED.hpp"

OLED::OLED(const uint8_t &width, const uint8_t &height, const uint8_t &display_address)
    : display(Adafruit_SSD1306(width, height)) {
    if(!this->display.begin(SSD1306_SWITCHCAPVCC, display_address)) {
        Serial.println(F("SSD1306 allocation failed"));
        esp_deep_sleep_start();
    }
    this->display.display();
    delay(2000);
    this->display.clearDisplay();
}

void OLED::wellcome() {
    this->display.clearDisplay();
    this->display.setTextColor(SSD1306_WHITE);

    this->display.setTextSize(2);
    this->display.setCursor(10, 10);
    this->display.println("AGRI ARENA");
    delay(2000);

    this->display.clearDisplay();
    this->display.setTextSize(0);
    this->display.setCursor(0, 0);
}

void OLED::message(const String &message) {}

void OLED::error(const String &error) {}

void OLED::reading() {}

void OLED::sending() {}
