#include "led_strip.h"

#include "config.h"
#include "PicoLed.hpp"

namespace led_strip
{
    auto& get_strip(int strip)
    {
        static auto led_strip_1 = PicoLed::addLeds<PicoLed::WS2812B>(
            pio1, 1, led_cfg->led.led_1.pin, led_cfg->led.led_1.count, led_cfg->led.led_1.format
        );

        static auto led_strip_2 = PicoLed::addLeds<PicoLed::WS2812B>(
            pio1, 2, led_cfg->led.led_2.pin, led_cfg->led.led_2.count, led_cfg->led.led_2.format
        );

        return (strip == 0) ? led_strip_1 : led_strip_2;
    }

    void reset(int strip)
    {
        auto& target = get_strip(strip);
        target.clear();
        target.show();
    }

    void set_pixels(const std::array<color, MAX_LEDS>& payload, int strip)
    {
        auto& target = get_strip(strip);
        auto& led_config = (strip == 0) ? led_cfg->led.led_1 : led_cfg->led.led_2;

        for (int i = 0; i < led_config.count; ++i)
            target.setPixelColor(i, PicoLed::RGB(payload[i].r, payload[i].g, payload[i].b));

        target.setBrightness(led_config.brightness);
        target.show();
    }

    void set_pixel(int pixel, color led, int strip)
    {
        auto& target = get_strip(strip);
        target.setPixelColor(pixel, PicoLed::RGB(led.r, led.g, led.b));
        target.show();
    }

    void fill_strip(int strip)
    {
        auto& target = get_strip(strip);
        target.fill(PicoLed::RGB(255, 0, 255));
        target.setBrightness(100);
        target.show();
    }

    void set_brightness(uint8_t brightness, int strip)
    {
        auto& target = get_strip(strip);
        target.setBrightness(brightness);
        target.show();
    }
}
