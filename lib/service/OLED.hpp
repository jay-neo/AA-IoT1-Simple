#ifndef OLED_HPP
#define OLED_HPP

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

class OLED {
    Adafruit_SSD1306 display;

   public:
    OLED(const uint8_t &width, const uint8_t &height, const uint8_t &screen_address);
    void wellcome();
    void message(const String &message);
    void error(const String &error);
    void reading();
    void sending();
};

#endif