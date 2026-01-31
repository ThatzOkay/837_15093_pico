//
// LED Emulator configg
//

#ifndef PICO_SEGA_LED_CONFIG_H
#define PICO_SEGA_LED_CONFIG_H

#include "effects.h"
#include "PicoLedTarget.hpp"

#define MAX_PACKET 256
#define UART_READ_TIMEOUT_US 2000
#define UART0_ID uart0
#define UART1_ID uart1

enum Mode {
    ARCADE,
    EFFECT
};

struct ModeConfig {
    Mode mode;
};

struct EffectConfig {
    Effect effect;
};

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
        Mode mode;
    } ledMode;
    struct {
        Effect effect;
    } ledEffect;
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
    struct
    {
        uint8_t chip_num[5];
        uint8_t board_name[8];
        uint16_t firm_sum;
    } firmware;
} led_cfg_t;

extern led_cfg_t *led_cfg;

led_cfg_t get_chuni_preset(bool airs);
led_cfg_t get_ongeki_preset();
void config_init();
void config_changed(); // Notify the config has changed
void config_factory_reset(); // Reset the config to factory default
#endif //PICO_SEGA_LED_CONFIG_H
