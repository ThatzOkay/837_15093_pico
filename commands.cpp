#include "cli.h"
#include "config.h"
#include "log.h"
#include "save.h"
#include "hardware/watchdog.h"

void disp_debug()
{
    cli_log("[Debug]\n");
    cli_log(" Enabled: %b \n", led_cfg->debug.enable);
}

const char* format_names[] = { "RGB", "GRB", "WRGB" };

void disp_led()
{
    cli_log("[LED]\n");
    cli_log("Test enabled: %b \n", led_cfg->led.enable_test);
    cli_log("Led strip 1: Pin: %u, Count: %u, Offset: %u, Brightness: %u, Format: %s \n", led_cfg->led.led_1.pin,
            led_cfg->led.led_1.count, led_cfg->led.led_1.offset, led_cfg->led.led_1.brightness,
            format_names[led_cfg->led.led_1.format]);
    cli_log("Led strip 2: Pin: %u, Count: %u, Offset: %u, Brightness: %u, Format: %s \n", led_cfg->led.led_2.pin,
            led_cfg->led.led_2.count, led_cfg->led.led_2.offset, led_cfg->led.led_2.brightness,
            format_names[led_cfg->led.led_2.format]);
}

void disp_fade()
{
    cli_log("[Fade]\n");
    cli_log("Delay: %u\n", led_cfg->fade.delay);
}

void disp_uart()
{
    cli_log("[UART]\n");
    cli_log("UART enabled: %u \n", led_cfg->uart.enable);
    cli_log("UART 0: Pin RX: %u, PIN TX: %u\n", led_cfg->uart.uart_0_pin.rx, led_cfg->uart.uart_0_pin.tx);
    cli_log("UART 1: Pin RX: %u, PIN TX: %u\n", led_cfg->uart.uart_1_pin.rx, led_cfg->uart.uart_1_pin.tx);
}

void handle_display(int argc, char* argv[])
{
    auto usage = "Usage: display [debug|led|fade|uart]\n";

    if (argc > 1)
    {
        cli_log(usage);
    }

    if (argc == 0)
    {
        disp_debug();
        disp_led();
        disp_fade();
        disp_uart();
    }

    const char* choices[] = {"debug", "led", "fade", "uart"};
    switch (cli_match_prefix(choices, count_of(choices), argv[0]))
    {
    case 0:
        disp_debug();
        break;
    case 1:
        disp_led();
        break;
    case 2:
        disp_fade();
        break;
    case 3:
        disp_uart();
        break;
    default:
        cli_log(usage);
        break;
    }
}

static void handle_debug(int argc, char* argv[])
{
    const char* usage = "Usage: debug [on|off]\n";

    if (argc != 1)
    {
        cli_log("%s", usage);
        return;
    }

    const char* on_off[] = {"off", "on"};
    int on = cli_match_prefix(on_off, count_of(on_off), argv[0]);
    if (on < 0)
    {
        cli_log("%s", usage);
        return;
    }

    led_cfg->debug.enable = on;
    config_changed();
}

auto led_usage =
    "Usage: led test<on|off> | <1|2>\n [pin|count|offset|brightness<0-255>|(format:[rgb,grb,wrgb])]\n";

void handle_led_cfg(int led_strip, int argc, char* argv[])
{
    auto& led_config = led_strip == 1 ? led_cfg->led.led_1 : led_cfg->led.led_2;

    if (argc < 3)
    {
        cli_log("%s", led_usage);
        return;
    }

    const char* keywords[] = { "pin", "count", "offset", "brightness", "format" };
    const int match = cli_match_prefix(keywords, count_of(keywords), argv[1]);

    if (match < 0)
    {
        cli_log("%s", led_usage);
        return;
    }

    if (match != 4)
    {
        const int value = cli_extract_non_neg_int(argv[2], 0);
        if (value > 255)
        {
            cli_log(led_usage);
            return;
        }

        switch (match)
        {
        case 0: led_config.pin = value; break;
        case 1: led_config.count = value; break;
        case 2: led_config.offset = value; break;
        case 3: led_config.brightness = value; break;
        default:
            cli_log(led_usage);
        }
    }
    else
    {
        const char* formats[] = { "rgb", "grb", "wrgb" };
        const int formatMatch = cli_match_prefix(formats, count_of(formats), argv[2]);
        if (formatMatch < 0)
        {
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

static void handle_led(int argc, char* argv[])
{
    if (argc < 1)
    {
        cli_log("%s", led_usage);
        return;
    }

    const char* commands[] = {"test", "1", "2"};
    int match = cli_match_prefix(commands, count_of(commands), argv[0]);

    if (match < 0)
    {
        cli_log("%s", led_usage);
        return;
    }

    const char* on_off[] = {"on", "off"};
    int on;

    switch (match)
    {
    case 0:

        if (argc < 2)
        {
            cli_log("%s", led_usage);
            break;
        }

        on = cli_match_prefix(on_off, count_of(on_off), argv[1]);
        if (on < 0)
        {
            cli_log("%s", led_usage);
            break;
        }

        led_cfg->led.enable_test = (on > 0);
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

void handle_fade(int argc, char* argv[])
{
    const char* usage = "Usage: fade <0..255> [delay in ms]\n";

    if (argc != 1)
    {
        cli_log("%s", usage);
        return;
    }

    int delay = cli_extract_non_neg_int(argv[0], 0);

    if ((delay < 0) || (delay > 255))
    {
        cli_log(usage);
        return;
    }

    led_cfg->fade.delay = delay;
    config_changed();
}

auto uart_usage =
    "Usage: uart enable<on|off> | <uart: 0|1>\n [tx|rx]\n";

void handle_uart_cfg(int uart, int argc, char* argv[])
{
    auto* uart_cfg = (uart == 0) ? &led_cfg->uart.uart_0_pin : &led_cfg->uart.uart_1_pin;

    if (argc < 3)
    {
        cli_log("%s", uart_usage);
        return;
    }

    const char* keywords[] = {"tx", "rx"};

    int match = cli_match_prefix(keywords, count_of(keywords), argv[1]);
    if (match < 0)
    {
        cli_log("%s", uart_usage);
        return;
    }

    int value = cli_extract_non_neg_int(argv[2], 0);

    switch (match)
    {
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

void handle_uart(int argc, char* argv[])
{
    if (argc < 1)
    {
        cli_log(uart_usage);
        return;
    }

    const char* commands[] = {"enable", "0", "1"};
    int match = cli_match_prefix(commands, count_of(commands), argv[0]);

    if (match < 0)
    {
        cli_log(uart_usage);
        return;
    }

    const char* on_off[] = {"on", "off"};
    int on;

    switch (match)
    {
    case 0:

        if (argc < 2)
        {
            cli_log(uart_usage);
            break;
        }

        on = cli_match_prefix(on_off, count_of(on_off), argv[1]);
        if (on < 0)
        {
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
    default:
        cli_log(uart_usage);
        return;
    }

    config_changed();
    cli_log("It is advised to issue a reboot command after saving is finished to initialize new pin values.\n");
}

static void handle_save()
{
    save_request(true);
}

static void handle_factory_reset()
{
    config_factory_reset();
    debug_log("Factory reset done.\n");
}

static void handle_reboot()
{
    watchdog_reboot(0, 0, 0);
}


void commands_init()
{
    cli_register("display", cmd_handler_t(handle_display), "Display all config.");
    cli_register("debug", cmd_handler_t(handle_debug), "Set debug config.");
    cli_register("led", cmd_handler_t(handle_led), "Set led config.");
    cli_register("fade", cmd_handler_t(handle_fade), "Set fade config.");
    cli_register("uart", cmd_handler_t(handle_uart), "Set UART config.");
    cli_register("save", cmd_handler_t(handle_save), "Save config to flash.");
    cli_register("factory", cmd_handler_t(handle_factory_reset), "Reset everything to default.");
    cli_register("reboot", cmd_handler_t(handle_reboot), "Reboot the pico");
}
