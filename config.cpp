#include "config.h"

#include "save.h"

led_cfg_t *led_cfg;

static led_cfg_t default_cfg = {
    .debug = {
        .enable = false
    },
    .ledMode = { ARCADE },
    .ledEffect = { BOUNCE },
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
    },
    .firmware = {
        .chip_num = { '6', '7', '1', '0', ' '},
        .board_name = {'1','5','0','9','3','-','0','6'},
        .firm_sum = 0xADF7
    }
};

static led_cfg_t preset_chuni = {
    .debug = {
        .enable = false
    },
    .ledMode = { ARCADE },
    .ledEffect = { BOUNCE },
    .led = {
        .enable_test = false,
        .led_1 = {
            .pin = 16,
            .count = 53,
            .offset = 0,
            .brightness = 200,
            .format = PicoLed::FORMAT_GRB
        },
        .led_2 = {
            .pin = 15,
            .count = 63,
            .offset = 0,
            .brightness = 200,
            .format = PicoLed::FORMAT_GRB
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
    },
    .firmware = {
        .chip_num = { '6', '7', '1', '0', ' '},
        .board_name = {'1','5','0','9','3','-','0','6'},
        .firm_sum = 0xADF7
    }
};
static led_cfg_t preset_chuni_airs = {
    .debug = {
        .enable = false
    },
    .led = {
        .enable_test = false,
        .led_1 = {
            .pin = 16,
            .count = 3,
            .offset = 50,
            .brightness = 200,
            .format = PicoLed::FORMAT_GRB
        },
        .led_2 = {
            .pin = 15,
            .count = 3,
            .offset = 60,
            .brightness = 200,
            .format = PicoLed::FORMAT_GRB
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
    },
    .firmware = {
        .chip_num = { '6', '7', '1', '0', ' '},
        .board_name = {'1','5','0','9','3','-','0','6'},
        .firm_sum = 0xADF7
    }
};

static led_cfg_t preset_ongeki = {
    .debug = {
        .enable = false
    },
    .ledMode = { ARCADE },
    .ledEffect = { BOUNCE },
    .led = {
        .enable_test = false,
        .led_1 = {
            .pin = 16,
            .count = 61,
            .offset = 0,
            .brightness = 200,
            .format = PicoLed::FORMAT_GRB
        },
        .led_2 = {
            .pin = 15,
            .count = 64,
            .offset = 0,
            .brightness = 200,
            .format = PicoLed::FORMAT_GRB
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
    },
    .firmware = {
        .chip_num = { '6', '7', '1', '0', 'A'},
        .board_name = {'1','5','0','9','3','-','0','6'},
        .firm_sum = 0xAA53
    }
};

static void config_loaded()
{

}

led_cfg_t get_chuni_preset(bool airs)
{
    if (airs)
        return preset_chuni_airs;
    return preset_chuni;
}

led_cfg_t get_ongeki_preset()
{
    return preset_ongeki;
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
