#include "led_strip.h"

#include "config.h"
#include "PicoLed.hpp"

namespace led_strip
{
    auto led_strip_1 = PicoLed::addLeds<PicoLed::WS2812B>(pio1, 1, LED_STRIP_01, 64, PicoLed::FORMAT_GRB);
    auto led_strip_2 = PicoLed::addLeds<PicoLed::WS2812B>(pio1 , 2, LED_STRIP_02, 64, PicoLed::FORMAT_GRB);

    void reset(const int strip)
    {
        auto& target = (strip == 0) ? led_strip_1 : led_strip_2;

        target.clear();
        target.show();
    }

    void set_pixels(const std::array<color, MAX_LEDS>& payload, int strip)
    {
        auto& target = (strip == 0) ? led_strip_1 : led_strip_2;
        for (int i = 0; i < MAX_LEDS; ++i)
            target.setPixelColor(i, PicoLed::RGB(payload[i].r, payload[i].g, payload[i].b));
        target.show();
    }

    void set_pixel(int pixel, color led, int strip)
    {        
        auto& target = (strip == 0) ? led_strip_1 : led_strip_2;
        target.setPixelColor(pixel, PicoLed::RGB(led.r, led.g, led.b));
        target.show();
    }


    void fill_strip(int strip)
    {
        auto& target = (strip == 0) ? led_strip_1 : led_strip_2;
        target.fill(PicoLed::RGB(255, 0, 255));
        target.setBrightness(100);
        target.show();
    }
}
