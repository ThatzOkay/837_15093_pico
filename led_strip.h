#pragma once
#include <array>
#include <cstdint>

constexpr std::size_t MAX_LEDS = 64;

struct led
{
    uint8_t r, g, b;
};

namespace led_strip
{
    void reset(int strip);
    void set_pixels(const std::array<led, MAX_LEDS>& payload, int strip);
}
