//
// LED Emulator configg
//

#ifndef PICO_SEGA_LED_CONFIG_H
#define PICO_SEGA_LED_CONFIG_H
#include <cstdint>

#include "PicoLedTarget.hpp"

#define MAX_PACKET 256

struct LedConfig {
    uint8_t pin;
    uint8_t count;
    uint8_t offset;
    uint8_t brightness;
    PicoLed::DataFormat format;
};

struct UartConfig
{
    uint8_t tx;
    uint8_t rx;
};

typedef struct __attribute__((__packed__)) {
    struct {
        bool enable;
    } debug;
    struct {
        bool enable_test;
        LedConfig led_1;
        LedConfig led_2;
    } led;
    struct
    {
        uint8_t delay;
    } fade;
    struct
    {
        bool enable;
        UartConfig uart_0_pin;
        UartConfig uart_1_pin;
        uint32_t baud_rate;
    } uart;
} led_cfg_t;

extern led_cfg_t *led_cfg;

void config_init();
void config_changed(); // Notify the config has changed
void config_factory_reset(); // Reset the config to factory default
#endif //PICO_SEGA_LED_CONFIG_H
