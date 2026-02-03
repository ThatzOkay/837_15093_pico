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
    std::unique_ptr<PicoLed::PicoLedEffect> activeEffects[2];
    Effect currentEffects[2] = {NONE, NONE};

    const vector<PicoLed::Color> rainbowPallet = {
        {255, 0, 0},
        {255, 127, 0},
        {255, 255, 0},
        {0, 255, 0},
        {0, 0, 255},
        {75, 0, 130},
        {148, 0, 211}};

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
        if (strip < 0 || strip > 1)
            return;

        auto &target = get_strip(strip);

        activeEffects[strip].reset();

        switch (effect)
        {
        case BOUNCE:
        {
            PicoLed::Bounce bounceEffect(target, 1.5, 8.0);
            bounceEffect.addBall(PicoLed::RGB(255, 0, 0), 2.0);
            bounceEffect.addBall(PicoLed::RGB(0, 255, 0), 2.0);
            bounceEffect.addBall(PicoLed::RGB(0, 0, 255), 2.0);
            activeEffects[strip] = std::make_unique<PicoLed::Bounce>(bounceEffect);
            break;
        }
        case COMET:
            activeEffects[strip] = std::make_unique<PicoLed::Comet>(
                target, PicoLed::RGB(255, 255, 255), 10.0, 2.0, 1.5);
            break;
        case FADE:
            activeEffects[strip] = std::make_unique<PicoLed::Fade>(
                target, PicoLed::RGB(255, 255, 255), 5);
            break;
        case MARQUEE:
            activeEffects[strip] = std::make_unique<PicoLed::Marquee>(
                target, rainbowPallet, 1.0, 8.0);
            break;
        case PARTICLES:
        {
            PicoLed::Particles particlesEffect(target, rainbowPallet, 0.5, 1.5);
            particlesEffect.addSource(16, 0.75, -0.3);
            particlesEffect.addSource(17, 0.75, 0.3);
            activeEffects[strip] = std::make_unique<PicoLed::Particles>(particlesEffect);
            break;
        }
        case STARS:
            activeEffects[strip] = std::make_unique<PicoLed::Stars>(
                target, PicoLed::RGB(255, 255, 255), 4.0);
            break;
        case NONE:
            target.clear();
            target.show();
            break;
        default:
            return;
        }

        currentEffects[strip] = effect;

        if (activeEffects[strip])
        {
            activeEffects[strip]->animate();
            if (strip == 0)
                target.setBrightness(led_cfg->led.led_1.brightness);
            else
                target.setBrightness(led_cfg->led.led_2.brightness);
            target.show();
        }
    }

    void update_effect(int strip)
    {
        if (strip < 0 || strip > 1)
            return;
        if (!activeEffects[strip])
            return;

        auto &target = get_strip(strip);
        if (activeEffects[strip]->animate())
        {
            target.show();
        }
    }

    void update_effect(int strip, uint32_t timeGone)
    {
        if (strip < 0 || strip > 1)
            return;
        update_effect(strip);
    }

    bool should_update_effect(int strip, Effect newEffect)
    {
        if (strip < 0 || strip > 1)
            return false;
        return currentEffects[strip] != newEffect;
    }
}
