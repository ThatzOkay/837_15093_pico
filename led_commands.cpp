#include "led_commands.h"
#include "config.h"
#include <cstring>
#include "led_strip.h"
#include "log.h"

void led_get_board_info(jvs_req_any* req, jvs_resp_any* resp)
{
    resp->len += 18;
    resp->report = 1;
    memcpy(resp->payload , led_cfg->firmware.board_name, sizeof(led_cfg->firmware.board_name));

    *(resp->payload + 8) = 0x0A;

    memcpy(resp->payload + 9, led_cfg->firmware.chip_num, sizeof(led_cfg->firmware.chip_num));

    *(resp->payload + 14) = 0xFF;

    if (req->dest == 0x01 && req->src == 0x02)
    {
        *(resp->payload + 15) = 0xa0;
    }
    else
    {
        *(resp->payload + 15) = 0x90;
    }
    *(resp->payload + 16) = 0;
    *(resp->payload + 17) = 204;
}

void led_get_firm_sum(jvs_req_any* req, jvs_resp_any* resp)
{
    resp->len += 2;

    *(resp->payload) = (led_cfg->firmware.firm_sum >> 8) & 0xff;
    *(resp->payload + 1) = static_cast<uint8_t>(led_cfg->firmware.firm_sum) & 0xff;
}

void led_get_protocol_ver(jvs_req_any* req, jvs_resp_any* resp)
{
    resp->len += 3;
    *(resp->payload) = 0x01;
    *(resp->payload + 1) = 0x01;
    *(resp->payload + 2) = 0x00;
}

void led_reset(jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    // setting_timeout = 0;
    if (led_board == 0)
    {
        setting_disable_resp_1 = false;
        fade_mode_1 = FADE_MODE_NONE;
        fade_timer_1 = FADE_TIMER_DEFAULT;
        fade_modifier_1 = 1;
        fade_value_1 = 255;
    }
    else
    {
        setting_disable_resp_2 = false;
    }

    led_strip::reset(led_board);
}

void internal_led_set(jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    size_t offset = 0;
    switch (led_board)
    {
    case 0:
        offset = led_cfg->led.led_1.offset;
        break;
    case 1:
        offset = led_cfg->led.led_2.offset;
        break;
    default:
        offset = 0;
        break;
    }

    size_t num_leds = req->len / 3;

    if (num_leds > MAX_LEDS)
        num_leds = MAX_LEDS;

    std::array<color, MAX_LEDS> leds{};

    for (size_t i = 0; i < MAX_LEDS; ++i)
    {
        leds[i].r = 0;
        leds[i].g = 0;
        leds[i].b = 0;
    }

    for (size_t i = 0; i < num_leds; ++i)
    {
        leds[i].r = req->payload[(i + offset) * 3 + 0];
        leds[i].g = req->payload[(i + offset) * 3 + 1];
        leds[i].b = req->payload[(i + offset) * 3 + 2];
    }

    led_strip::set_pixels(leds, led_board);
    led_strip::set_brightness(200, led_board);
}

void led_set(jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    if (led_board == 0)
    {
        fade_mode_1 = FADE_MODE_NONE;
        fade_timer_1 = FADE_TIMER_DEFAULT;
        fade_modifier_1 = 1;
        fade_value_1 = 255;
    }

    internal_led_set(req, resp, led_board);
}

void led_set_fade(jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    if (led_board == 0)
    {
        fade_mode_1 = FADE_MODE_DECREASE;
        fade_value_1 = 200;
    }
    internal_led_set(req, resp, led_board);
}

void led_set_fade_pattern(jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    uint8_t depth = req->payload[0];
    uint8_t cycle = req->payload[1];

    if (led_board == 0)
    {
        fade_mode_1 = FADE_MODE_DECREASE;
        fade_value_1 = 255;

        fade_timer_1 = depth;
        fade_modifier_1 = cycle;
    }
}

void led_disable_response(jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    if (led_board == 0)
    {
        setting_disable_resp_1 = req->payload[0];
    }
    else
    {
        setting_disable_resp_2 = req->payload[0];
    }

    resp->len += 1;
    *(resp->payload) = req->payload[0];
}

void led_timeout(const jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    resp->len += 2;

    if (led_board == 0)
    {
        settings_timeout1 = req->payload[0] << 8 | req->payload[1];
    }
    else
    {
        settings_timeout2 = req->payload[0] << 8 | req->payload[1];
    }

    *(resp->payload) = req->payload[0];
    *(resp->payload + 1) = req->payload[1];
}

void led_get_board_status(jvs_req_any* req, jvs_resp_any* resp)
{
    resp->len += 4;
    // is this needed??
    *(resp->payload) = 0;
    *(resp->payload + 1) = 0;
    *(resp->payload + 2) = 0;
    *(resp->payload + 3) = 0;
}

void handle_led_command(jvs_req_any* req, jvs_resp_any* resp, int led_board)
{
    resp->src = req->dest;
    resp->dest = req->src;
    resp->cmd = req->cmd;
    resp->len = 3;
    resp->status = 1;
    resp->report = 1;

    switch (req->cmd)
    {
    case LED_CMD_GET_BOARD_INFO:
        led_get_board_info(req, resp);
        break;

    case LED_CMD_GET_FIRM_SUM:
        led_get_firm_sum(req, resp);
        break;

    case LED_CMD_GET_PROTOCOL_VER:
        led_get_protocol_ver(req, resp);
        break;

    case LED_CMD_DISABLE_RESPONSE:
        led_disable_response(req, resp, led_board);
        break;

    case LED_CMD_RESET:
        led_reset(req, resp, led_board);
        break;
    case LED_CMD_SET_LED:
        led_set(req, resp, led_board);
        break;

    case LED_CMD_SET_LED_FADE:
        led_set_fade(req, resp, led_board);
        break;

    case LED_CMD_SET_LED_FADE_PATTERN:
        led_set_fade_pattern(req, resp, led_board);
        break;

    case LED_CMD_TIMEOUT:
        led_timeout(req, resp, led_board);
        break;

    case LED_CMD_GET_BOARD_STATUS:
        led_get_board_status(req, resp);
        break;

    default:
        debug_log("Unknown command: 0x%02X\n", req->cmd);
        break;
    }
}