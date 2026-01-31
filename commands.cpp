#include <cstring>

#include "cli.h"
#include "config.h"
#include "log.h"
#include "save.h"
#include "hardware/watchdog.h"

void disp_debug() {
    cli_log("[Debug]\n");
    cli_log(" Enabled: %b \n", led_cfg->debug.enable);
}

void disp_mode() {
    cli_log("[Mode]\n");
    const auto mode = led_cfg->ledMode.mode == ARCADE ? "Arcade" : "Effect";
    cli_log("Mode: %s\n", mode);
}

void disp_effect() {
    cli_log("[Effect]\n");
    const auto effect = toString(led_cfg->ledEffect.effect);
    cli_log("Effect: %s\n", effect);
}

const char *format_names[] = {"RGB", "GRB", "WRGB"};

void disp_led() {
    cli_log("[LED]\n");
    cli_log("Test enabled: %b \n", led_cfg->led.enable_test);
    cli_log("Led strip 1: Pin: %u, Count: %u, Offset: %u, Brightness: %u, Format: %s \n", led_cfg->led.led_1.pin,
            led_cfg->led.led_1.count, led_cfg->led.led_1.offset, led_cfg->led.led_1.brightness,
            format_names[led_cfg->led.led_1.format]);
    cli_log("Led strip 2: Pin: %u, Count: %u, Offset: %u, Brightness: %u, Format: %s \n", led_cfg->led.led_2.pin,
            led_cfg->led.led_2.count, led_cfg->led.led_2.offset, led_cfg->led.led_2.brightness,
            format_names[led_cfg->led.led_2.format]);
}

void disp_fade() {
    cli_log("[Fade]\n");
    cli_log("Delay: %u\n", led_cfg->fade.delay);
}

void disp_uart() {
    cli_log("[UART]\n");
    cli_log("UART enabled: %u \n", led_cfg->uart.enable);
    cli_log("UART 0: Pin RX: %u, PIN TX: %u\n", led_cfg->uart.uart_0_pin.rx, led_cfg->uart.uart_0_pin.tx);
    cli_log("UART 1: Pin RX: %u, PIN TX: %u\n", led_cfg->uart.uart_1_pin.rx, led_cfg->uart.uart_1_pin.tx);
    cli_log("Baud rate: %u\n", led_cfg->uart.baud_rate);
}

void disp_firmware() {
    const auto chip_num = led_cfg->firmware.chip_num;
    const auto board_name = led_cfg->firmware.board_name;

    cli_log("[Firmware]\n");
    cli_log("Firmware: Chip number: %.*s\n", 5, (const char *) chip_num);
    cli_log("Firmware: Board name: %.*s\n", 8, (const char *) board_name);
    cli_log("Firmware: Firmware sum: 0x%X\n", led_cfg->firmware.firm_sum);
}

void handle_display(int argc, char *argv[]) {
    auto usage = "Usage: display [debug|led|fade|uart]\n";

    if (argc > 1) {
        cli_log(usage);
    }

    if (argc == 0) {
        disp_debug();
        disp_mode();
        disp_effect();
        disp_led();
        disp_fade();
        disp_uart();
        disp_firmware();
    }
    
    const char *choices[] = {"debug", "mode", "effect", "led", "fade", "uart", "firmware"};
    switch (cli_match_prefix(choices, std::size(choices), argv[0])) {
        case 0:
            disp_debug();
            break;
        case 1:
            disp_mode();
            break;
        case 2:
            disp_effect();
            break;
        case 3:
            disp_led();
            break;
        case 4:
            disp_fade();
            break;
        case 5:
            disp_uart();
            break;
        case 6:
            disp_firmware();
            break;
        default:
            cli_log(usage);
            break;
    }
}

static void handle_debug(const int argc, char *argv[]) {
    auto usage = "Usage: debug [on|off]\n";

    if (argc != 1) {
        cli_log("%s", usage);
        return;
    }

    const char *on_off[] = {"off", "on"};
    const int on = cli_match_prefix(on_off, std::size(on_off), argv[0]);
    if (on < 0) {
        cli_log("%s", usage);
        return;
    }

    led_cfg->debug.enable = on;
    config_changed();
}

auto mode_usage =
        "Usage: mode <arcade|effect>\n";

static void handle_mode(const int argc, char *argv[]) {
    if (argc != 1) {
        cli_log("%s", mode_usage);
        return;
    }

    const char *keywords[] = {"arcade", "effect"};
    const int match = cli_match_prefix(keywords, std::size(keywords), argv[1]);
    if (match < 0) {
        cli_log("%s", mode_usage);
        return;
    }

    switch (match) {
        case 0:
            led_cfg->ledMode.mode = ARCADE;
            break;
        case 1:
            led_cfg->ledMode.mode = EFFECT;
            break;
        default:
            cli_log("%s", mode_usage);
            break;
    }
}


void log_effect_usage() {
    const Effect effects[] = {BOUNCE, COMET, FADE, MARQUEE, PARTICLES, STARS};
    cli_log("Usage: effect <");

    for (size_t i = 0; i < sizeof(effects)/sizeof(effects[0]); ++i) {
        cli_log("%s", toString(effects[i]));
        if (i != sizeof(effects)/sizeof(effects[0]) - 1) {
            cli_log("|");
        }
    }

    cli_log(">\n");
}

static void handle_effect(const int argc, char *argv[]) {


    if (led_cfg->ledMode.mode == ARCADE) {
        cli_log("Please first enable effect mode before setting led effect \n");
        return;
    }

    if (argc != 1) {
        log_effect_usage();
        return;
    }

    const int match = cli_match_prefix(effectNames, std::size(effectNames), argv[1]);
    if (match < 0) {
        log_effect_usage();
        return;
    }

    switch (match) {
        case 0:
            led_cfg->ledEffect.effect = BOUNCE;
            break;
        case 1:
            led_cfg->ledEffect.effect = COMET;
            break;
        case 2:
            led_cfg->ledEffect.effect = FADE;
            break;
        case 3:
            led_cfg->ledEffect.effect = MARQUEE;
            break;
        case 4:
            led_cfg->ledEffect.effect = PARTICLES;
            break;
        case 5:
            led_cfg->ledEffect.effect = STARS;
            break;
         default:
            log_effect_usage();
    }
}

auto led_usage =
        "Usage: led test<on|off> | <1|2>\n [pin|count|offset|brightness<0-255>|(format:[rgb,grb,wrgb])]\n";

void handle_led_cfg(int led_strip, int argc, char *argv[]) {
    auto &led_config = led_strip == 1 ? led_cfg->led.led_1 : led_cfg->led.led_2;

    if (argc < 3) {
        cli_log("%s", led_usage);
        return;
    }

    const char *keywords[] = {"pin", "count", "offset", "brightness", "format"};
    const int match = cli_match_prefix(keywords, std::size(keywords), argv[1]);

    if (match < 0) {
        cli_log("%s", led_usage);
        return;
    }

    if (match != 4) {
        const int value = cli_extract_non_neg_int(argv[2], 0);
        if (value > 255) {
            cli_log(led_usage);
            return;
        }

        switch (match) {
            case 0: led_config.pin = value;
                break;
            case 1: led_config.count = value;
                break;
            case 2: led_config.offset = value;
                break;
            case 3: led_config.brightness = value;
                break;
            default:
                cli_log(led_usage);
        }
    } else {
        const char *formats[] = {"rgb", "grb", "wrgb"};
        const int formatMatch = cli_match_prefix(formats, std::size(formats), argv[2]);
        if (formatMatch < 0) {
            cli_log(led_usage);
            return;
        }

        static constexpr PicoLed::DataFormat format_values[] = {
            PicoLed::FORMAT_RGB,
            PicoLed::FORMAT_GRB,
            PicoLed::FORMAT_WRGB
        };

        led_config.format = format_values[formatMatch];
    }
}

static void handle_led(int argc, char *argv[]) {
    if (argc < 1) {
        cli_log("%s", led_usage);
        return;
    }

    const char *commands[] = {"test", "1", "2"};
    int match = cli_match_prefix(commands, count_of(commands), argv[0]);

    if (match < 0) {
        cli_log("%s", led_usage);
        return;
    }

    const char *on_off[] = {"on", "off"};
    int on;

    switch (match) {
        case 0:

            if (argc < 2) {
                cli_log("%s", led_usage);
                break;
            }

            on = cli_match_prefix(on_off, count_of(on_off), argv[1]);
            if (on < 0) {
                cli_log("%s", led_usage);
                break;
            }

            led_cfg->led.enable_test = on == 0;
            break;
        case 1:
            handle_led_cfg(1, argc, argv);
            break;
        case 2:
            handle_led_cfg(2, argc, argv);
            break;
        default:
            cli_log("%s", led_usage);
            return;
    }

    config_changed();
    cli_log("It is advised to issue a reboot command after saving is finished to initialize new pin values.\n");
}

void handle_fade(int argc, char *argv[]) {
    const char *usage = "Usage: fade <0..255> [delay in ms]\n";

    if (argc != 1) {
        cli_log("%s", usage);
        return;
    }

    int delay = cli_extract_non_neg_int(argv[0], 0);

    if ((delay < 0) || (delay > 255)) {
        cli_log(usage);
        return;
    }

    led_cfg->fade.delay = delay;
    config_changed();
}

auto uart_usage =
        "Usage: uart enable <on|off> | baud_rate <value> \n"
        "       uart <uart: 0|1> [tx|rx] <value>\n"
        "Supported baud rates: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200\n";

void handle_uart_cfg(int uart, int argc, char *argv[]) {
    auto *uart_cfg = (uart == 0) ? &led_cfg->uart.uart_0_pin : &led_cfg->uart.uart_1_pin;

    if (argc < 3) {
        cli_log("%s", uart_usage);
        return;
    }

    const char *keywords[] = {"tx", "rx"};

    int match = cli_match_prefix(keywords, count_of(keywords), argv[1]);
    if (match < 0) {
        cli_log("%s", uart_usage);
        return;
    }

    int value = cli_extract_non_neg_int(argv[2], 0);

    switch (match) {
        case 0:
            uart_cfg->tx = value;
            break;
        case 1:
            uart_cfg->rx = value;
            break;
        default:
            cli_log("%s", uart_usage);
            break;
    }
}

const int supported_baud_rates[] = {
    1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200
};
constexpr int num_supported_baud_rates = std::size(supported_baud_rates);

void handle_uart(const int argc, char *argv[]) {
    if (argc < 1) {
        cli_log(uart_usage);
        return;
    }

    const char *commands[] = {"enable", "0", "1", "baud_rate"};
    int match = cli_match_prefix(commands, std::size(commands), argv[0]);

    if (match < 0) {
        cli_log(uart_usage);
        return;
    }

    const char *on_off[] = {"on", "off"};
    int on;

    switch (match) {
        case 0:

            if (argc < 2) {
                cli_log(uart_usage);
                break;
            }

            on = cli_match_prefix(on_off, std::size(on_off), argv[1]);
            if (on < 0) {
                cli_log(uart_usage);
                break;
            }

            led_cfg->uart.enable = on == 0;
            break;
        case 1:
            handle_uart_cfg(0, argc, argv);
            break;
        case 2:
            handle_uart_cfg(1, argc, argv);
            break;
        case 3: {
            const int value = cli_extract_non_neg_int(argv[1], 0);

            bool valid = false;
            for (int supported_baud_rate: supported_baud_rates) {
                if (value == supported_baud_rate) {
                    valid = true;
                    break;
                }
            }

            if (!valid) {
                cli_log("Invalid baud rate. Supported rates are: ");
                for (int i = 0; i < num_supported_baud_rates; i++) {
                    cli_log("%d%s", supported_baud_rates[i], (i < num_supported_baud_rates - 1) ? ", " : "\n");
                }
                break;
            }

            led_cfg->uart.baud_rate = value;
            break;
        }
        default:
            cli_log(uart_usage);
            return;
    }

    config_changed();
    cli_log("It is advised to issue a reboot command after saving is finished to initialize new pin values.\n");
}

void handle_firmware(int argc, char *argv[]) {
    const char *usage = "Usage: firmware chip_num: \" 5 chars\", board_name: \"string 8 chars\", sum: <0xsum>\n";

    if (argc < 1) {
        cli_log(usage);
        return;
    }

    const char *commands[] = {"chip_num", "board_name", "sum"};

    int match = cli_match_prefix(commands, std::size(commands), argv[0]);

    if (match < 0) {
        cli_log(usage);
        return;
    }

    switch (match) {
        case 0: {
            // Can't overflow
            if (std::strlen(argv[1]) < 5) {
                cli_log("Chip num  name too short \n");
                return;
            }
            const std::string value = cli_extract_non_neg_string(argv[1], 0);

            std::memset(led_cfg->firmware.chip_num, 0, sizeof(led_cfg->firmware.chip_num));
            std::memcpy(
                led_cfg->firmware.chip_num,
                value.c_str(),
                std::min(value.size(), sizeof(led_cfg->firmware.chip_num))
            );
            break;
        }
        case 1: {
            // Can't overflow
            if (std::strlen(argv[1]) < 8) {
                cli_log("Board name too short \n");
                return;
            }

            const std::string value = cli_extract_non_neg_string(argv[1], 0);

            std::memset(led_cfg->firmware.board_name, 0, sizeof(led_cfg->firmware.board_name));
            std::memcpy(
                led_cfg->firmware.board_name,
                value.data(),
                std::min(value.size(), sizeof(led_cfg->firmware.board_name))
            );
        }
        case 2: {
            const uint16_t value = cli_extract_non_neg_uint16(argv[1], 0);

            led_cfg->firmware.firm_sum = value;
        }
        default: ;
    }

    config_changed();
}

void handle_preset(int argc, char *argv[]) {
    const char *usage = "Usage: preset <chuni|chuni_airs|ongeki>\n";

    if (argc < 1) {
        cli_log("%s", usage);
        return;
    }

    const char *presets[] = {"chuni", "chuni_airs", "ongeki"};
    int match = cli_match_prefix(presets, std::size(presets), argv[0]);

    if (match < 0) {
        cli_log("%s", usage);
        return;
    }

    switch (match) {
        case 0:
            *led_cfg = get_chuni_preset(false);
            break;
        case 1:
            *led_cfg = get_chuni_preset(true);
            break;
        case 2:
            *led_cfg = get_ongeki_preset();
            break;
        default:
            cli_log("%s", usage);
            return;
    }

    config_changed();
}

static void handle_save() {
    save_request(true);
}

static void handle_factory_reset() {
    config_factory_reset();
    debug_log("Factory reset done.\n");
}

static void handle_reboot() {
    watchdog_reboot(0, 0, 0);
}


void commands_init() {
    cli_register("display", cmd_handler_t(handle_display), "Display all config.");
    cli_register("debug", cmd_handler_t(handle_debug), "Set debug config.");
    cli_register("mode", cmd_handler_t(handle_mode), "Set led mode.");
    cli_register("effect", cmd_handler_t(handle_effect), "Set led effect (Only works if mode is effect)");
    cli_register("led", cmd_handler_t(handle_led), "Set led config.");
    cli_register("fade", cmd_handler_t(handle_fade), "Set fade config.");
    cli_register("uart", cmd_handler_t(handle_uart), "Set UART config.");
    cli_register("firmware", cmd_handler_t(handle_firmware), "Set Firmware config.");
    cli_register("preset", cmd_handler_t(handle_preset), "Load preset config.");
    cli_register("save", cmd_handler_t(handle_save), "Save config to flash.");
    cli_register("factory", cmd_handler_t(handle_factory_reset), "Reset everything to default.");
    cli_register("reboot", cmd_handler_t(handle_reboot), "Reboot the pico");
}
