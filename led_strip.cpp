#include "led_strip.h"

#include "config.h"
#include "PicoLed.hpp"

namespace led_strip
{
    auto led_strip_1 = PicoLed::addLeds<PicoLed::WS2812B>(pio1, 1, LED_STRIP_01, 64, PicoLed::FORMAT_GRB);
    auto led_strip_2 = PicoLed::addLeds<PicoLed::WS2812B>(pio1 , 2, LED_STRIP_02, 64, PicoLed::FORMAT_GRB);

    void reset(const int strip)
    {
        if (strip == 0)
        {
            led_strip_1.clear();
        }
        else
        {
            led_strip_2.clear();
        }
    }

    void set_pixels(const std::array<led, MAX_LEDS>& payload, int strip)
    {
        auto& target = (strip == 0) ? led_strip_1 : led_strip_2;
        for (int i = 0; i < MAX_LEDS; ++i)
            target.setPixelColor(i, PicoLed::RGB(payload[i].r, payload[i].g, payload[i].b));
        target.show();
    }


}
