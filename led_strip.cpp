#include "led_strip.h"

#include "config.h"
#include "PicoLed.hpp"
#include "Effects/Bounce.hpp"
#include "Effects/Comet.hpp"
#include "Effects/Marquee.hpp"
#include "Effects/Particles.hpp"
#include "Effects/Stars.hpp"

namespace led_strip
{
    auto &get_strip(int strip)
    {
        static auto led_strip_1 = PicoLed::addLeds<PicoLed::WS2812B>(
            pio1, 1, led_cfg->led.led_1.pin, led_cfg->led.led_1.count, led_cfg->led.led_1.format);

        static auto led_strip_2 = PicoLed::addLeds<PicoLed::WS2812B>(
            pio1, 2, led_cfg->led.led_2.pin, led_cfg->led.led_2.count, led_cfg->led.led_2.format);

        return (strip == 0) ? led_strip_1 : led_strip_2;
    }

    void reset(int strip)
    {
        auto &target = get_strip(strip);
        target.clear();
        target.show();
    }

    void set_pixels(const std::array<color, MAX_LEDS> &payload, int strip)
    {
        auto &target = get_strip(strip);
        auto &led_config = (strip == 0) ? led_cfg->led.led_1 : led_cfg->led.led_2;

        for (int i = 0; i < led_config.count; ++i)
            target.setPixelColor(i, PicoLed::RGB(payload[i].r, payload[i].g, payload[i].b));

        target.setBrightness(led_config.brightness);
        target.show();
    }

    void set_pixel(int pixel, color led, int strip)
    {
        auto &target = get_strip(strip);
        target.setPixelColor(pixel, PicoLed::RGB(led.r, led.g, led.b));
        target.show();
    }

    void fill_strip(int strip)
    {
        auto &target = get_strip(strip);
        target.fill(PicoLed::RGB(255, 0, 255));
        target.setBrightness(100);
        target.show();
    }

    void set_brightness(uint8_t brightness, int strip)
    {
        auto &target = get_strip(strip);
        target.setBrightness(brightness);
        target.show();
    }

    void set_effect(int strip, Effect effect)
    {
        const vector<PicoLed::Color> rainbowPallet = {
            {255, 0, 0},
            {255, 127, 0},
            {255, 255, 0},
            {0, 255, 0},
            {0, 0, 255},
            {75, 0, 130},
            {148, 0, 211}
        };

        const PicoLed::Color baseColor{255, 255, 255, 255};

        auto &target = get_strip(strip);

        std::unique_ptr<PicoLed::PicoLedEffect> effectPtr;

        switch (effect)
        {
        case BOUNCE:
        {
            effectPtr = std::make_unique<PicoLed::Bounce>(target, 1, 1);
            break;
        }
        case COMET:
        {
            effectPtr = std::make_unique<PicoLed::Comet>(target, baseColor, 1, 1);
            break;
        }
        case FADE:
        {
            effectPtr = std::make_unique<PicoLed::Fade>(target, baseColor, 1);
            break;
        }
        case MARQUEE:
        {
            effectPtr = std::make_unique<PicoLed::Marquee>(target, rainbowPallet, 1, 1);
            break;
        }
        case PARTICLES:
        {
            effectPtr = std::make_unique<PicoLed::Particles>(target, rainbowPallet);
            break;
        }
        case STARS:
        {
            effectPtr = std::make_unique<PicoLed::Stars>(target, baseColor, 1);
            break;
        }
        default:
            break;
        }

        effectPtr->animate();
    }
}
