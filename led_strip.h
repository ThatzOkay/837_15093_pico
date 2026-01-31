#pragma once
#include <array>
#include <cstdint>

#include "effects.h"

constexpr std::size_t MAX_LEDS = 64;

struct color
{
    uint8_t r, g, b;
};

namespace led_strip
{
    void reset(int strip);
    void set_pixels(const std::array<color, MAX_LEDS>& payload, int strip);
    void set_pixel(int pixel, color led, int strip);
    void fill_strip(int strip);
    void set_brightness(uint8_t brightness, int strip);
    void set_effect(int strip, Effect effect);
}
