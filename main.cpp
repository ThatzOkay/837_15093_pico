#include <string>

#include "cli.h"
#include "commands.h"
#include "config.h"
#include "tusb.h"
#include "jvs.h"
#include "led_strip.h"
#include "led_commands.h"
#include "PicoLed.hpp"
#include "bsp/board_api.h"
#include "device/usbd.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "log.h"
#include "save.h"
#include "hardware/uart.h"
#include "port_handler.h"

static uint8_t jvs_buf_1[MAX_PACKET];
static uint32_t offset_1 = 0;

static uint8_t jvs_buf_2[MAX_PACKET];
static uint32_t offset_2 = 0;

void log_response(const jvs_resp_any* resp)
{
    debug_log("Response length: %d\nPayload: ", resp->len);
    for (int i = 0; i < resp->len; i++)
    {
        debug_log("%02X ", resp->payload[i]);
    }
    debug_log("\n");
}

bool is_all_zero(const void* buf, const size_t len)
{
    return memchr(buf, 1, len) == nullptr;
}

static mutex_t core1_io_lock;

[[noreturn]] static void core1_loop()
{
    while (true)
    {
        if (mutex_try_enter(&core1_io_lock, nullptr))
        {
            if (led_cfg->uart.enable)
            {
                process_uart_port(UART0_ID, jvs_buf_1, &offset_1);
                process_uart_port(UART1_ID, jvs_buf_2, &offset_2);
            }

            if (led_cfg->ledMode.mode == EFFECT) {
                led_strip::set_effect(0, led_cfg->ledEffect.effect);
                led_strip::set_effect(1, led_cfg->ledEffect.effect);
            }

            if (fade_mode_1 != 0)
            {
                debug_log("fading");
                if (fade_timer_1 > 0)
                {
                    fade_timer_1 -= led_cfg->fade.delay;
                }
                if (fade_timer_1 <= 0)
                {
                    if (fade_mode_1 == FADE_MODE_DECREASE)
                    {
                        fade_value_1 -= fade_modifier_1;
                        if (fade_value_1 <= 0)
                        {
                            fade_value_1 = 0;
                            fade_mode_1 = FADE_MODE_INCREASE;
                        }
                    }
                    else if (fade_mode_1 == FADE_MODE_INCREASE)
                    {
                        fade_value_1 += fade_modifier_1;
                        if (fade_value_1 >= 255)
                        {
                            fade_value_1 = 255;
                            fade_mode_1 = FADE_MODE_DECREASE;
                        }
                    }

                    debug_log("fade set");
                    led_strip::set_brightness(fade_value_1, 0);
                }
            }
            mutex_exit(&core1_io_lock);
        }
        sleep_us(10);
    }
}

[[noreturn]] static void core0_loop()
{
    uint64_t next_frame = time_us_64();
    while (true)
    {
        tud_task();

        if (tud_cdc_n_available(0))
        {
            process_cdc_port(0, jvs_buf_1, &offset_1);
        }

        if (tud_cdc_n_available(1))
        {
            process_cdc_port(1, jvs_buf_2, &offset_2);
        }

        cli_run();
        save_loop();

        next_frame = time_us_64() + 1000;
        sleep_until(next_frame);
    }
}

void led_test()
{
    led_strip::fill_strip(0);
    led_strip::fill_strip(1);

    sleep_ms(1000);
    led_strip::reset(0);
    led_strip::reset(1);
}

void init_uart()
{
    uart_init(UART0_ID, led_cfg->uart.baud_rate);

    gpio_set_function(led_cfg->uart.uart_0_pin.tx, UART_FUNCSEL_NUM(UART0_ID, led_cfg->uart.uart_0_pin.tx));
    gpio_set_function(led_cfg->uart.uart_0_pin.rx, UART_FUNCSEL_NUM(UART0_ID, led_cfg->uart.uart_0_pin.rx));

    uart_init(UART1_ID, led_cfg->uart.baud_rate);

    gpio_set_function(led_cfg->uart.uart_1_pin.tx, UART_FUNCSEL_NUM(UART1_ID, led_cfg->uart.uart_1_pin.tx));
    gpio_set_function(led_cfg->uart.uart_1_pin.rx, UART_FUNCSEL_NUM(UART1_ID, led_cfg->uart.uart_1_pin.rx));
}

void clean_uart()
{
    while (uart_is_readable(UART0_ID))
    {
        (void)uart_getc(UART0_ID);
    }

    uart_tx_wait_blocking(UART0_ID);

    while (uart_is_readable(UART1_ID))
    {
        (void)uart_getc(UART1_ID);
    }

    uart_tx_wait_blocking(UART1_ID);
}

void init()
{
    sleep_ms(50);
    set_sys_clock_khz(150000, true);
    board_init();
    tusb_init();
    stdio_init_all();

    config_init();

    mutex_init(&core1_io_lock);
    save_init(0xca34cafe, &core1_io_lock);

    cli_init("15093_pico>", "\n   << Sega LED Emulator >>\n"
             " https://github.com/thatzokay\n\n");

    commands_init();

    if (led_cfg->uart.enable)
    {
        init_uart();
        clean_uart();
    }
}

int main()
{
    init();
    if (led_cfg->led.enable_test)
    {
        led_test();
    }
    multicore_launch_core1(core1_loop);
    core0_loop();
}
