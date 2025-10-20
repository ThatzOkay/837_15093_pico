#include "config.h"

#include "save.h"

led_cfg_t *led_cfg;

static led_cfg_t default_cfg = {
    .debug = {
        .enable = false
    },
    .led = {
        .enable_test = false,
        .led_1 = {
            .pin = 16,
            .count = 64,
            .offset = 0,
            .brightness = 200,
            .format = PicoLed::FORMAT_RGB
        },
        .led_2 = {
            .pin = 15,
            .count = 64,
            .offset = 0,
            .brightness = 200,
            .format = PicoLed::FORMAT_RGB
        }
    },
    .fade = {
        .delay = 5
    },
    .uart = {
        .enable = false,
        .uart_0_pin = {
            .tx = 0,
            .rx = 1,
        },
        .uart_1_pin = {
            .tx = 0,
            .rx = 1,
        },
        .baud_rate = 115200
    }
};

static void config_loaded()
{

}

void config_changed()
{
    save_request(false);
}

void config_factory_reset()
{
    *led_cfg = default_cfg;
    save_request(true);
}

void config_init()
{
    led_cfg = static_cast<led_cfg_t*>(save_alloc(sizeof(*led_cfg), &default_cfg, config_loaded));
}
