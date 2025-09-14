#include <string>

#include "hresult.h"
#include "jvs.h"
#include "led.h"
#include "led_strip.h"
#include "PicoLed.hpp"
#include "bsp/board_api.h"
#include "device/usbd.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "pico/stdio.h"
#include "pico/time.h"
#include "pico/multicore.h"
#include "config.h"

#define FADE_TIMER_DEFAULT 80

#define FADE_MODE_NONE 0
#define FADE_MODE_DECREASE 1
#define FADE_MODE_INCREASE 2

static uint8_t fade_mode_1 = FADE_MODE_NONE;
static uint8_t fade_timer_1 = FADE_TIMER_DEFAULT;
static int16_t fade_value_1 = 255;
static int16_t fade_modifier_1 = 1;
static uint8_t settings_timeout1;
static bool setting_disable_resp_1 = false;
static uint8_t jvs_buf_1[MAX_PACKET];
static uint32_t offset_1 = 0;

static uint8_t settings_timeout2;
static bool setting_disable_resp_2 = false;
static uint8_t jvs_buf_2[MAX_PACKET];
static uint32_t offset_2 = 0;

static std::string chip_num_ongeki = "6710A";
static std::string chip_num_chuni = "6710 ";
static std::string board_name = "15093-06";

void log_response(const jvs_resp_any *resp)
{
	printf("Response length: %d\nPayload: ", resp->len);
	for (int i = 0; i < resp->len; i++)
	{
		printf("%02X ", resp->payload[i]);
	}
	printf("\n");
}

bool is_all_zero(const void *buf, size_t len)
{
	return memchr(buf, 1, len) == nullptr;
}

void led_get_board_info(jvs_req_any *req, jvs_resp_any *resp)
{
	resp->len += 18;
	resp->report = 1;
	strcpy(reinterpret_cast<char *>(resp->payload), board_name.c_str());
	*(resp->payload + 8) = 0x0A;

	if (req->dest == 0x01 && req->src == 0x02)
	{
		strcpy(reinterpret_cast<char *>(resp->payload) + 9, chip_num_ongeki.c_str());
	}
	else
	{
		strcpy(reinterpret_cast<char *>(resp->payload) + 9, chip_num_chuni.c_str());
	}
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

void led_get_firm_sum(jvs_req_any *req, jvs_resp_any *resp)
{
	resp->len += 2;

	if (req->dest == 0x01 && req->src == 0x02)
	{
		*(resp->payload) = (0xAA53 >> 8) & 0xff;
		*(resp->payload + 1) = (uint8_t)0xAA53 & 0xff;
	}
	else
	{
		*(resp->payload) = (0xADF7 >> 8) & 0xff;
		*(resp->payload + 1) = (uint8_t)0xADF7 & 0xff;
	}
}

void led_get_protocol_ver(jvs_req_any *req, jvs_resp_any *resp)
{
	resp->len += 3;
	*(resp->payload) = 0x01;
	*(resp->payload + 1) = 0x01;
	*(resp->payload + 2) = 0x00;
}

void led_reset(jvs_req_any *req, jvs_resp_any *resp, int led_board)
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

void internal_led_set(jvs_req_any *req, jvs_resp_any *resp, int led_board)
{
	size_t offset = 0;
	switch (led_board)
	{
	case 0:
		offset = LED_STRIP_01_OFFSET;
		break;
	case 1:
		offset = LED_STRIP_02_OFFSET;
		break;
	default:
		offset = 0;
		break;
	}

	size_t num_leds = req->len / 3;

	if (num_leds > MAX_LEDS)
		num_leds = MAX_LEDS;

	std::array<color, MAX_LEDS> leds{};

	if (offset > 0)
	{
		for (size_t i = 0; i < offset; ++i)
		{
			leds[i].r = 0;
			leds[i].g = 0;
			leds[i].b = 0;
		}
	}

	for (size_t i = 0; i < num_leds; ++i)
	{
		leds[i].r = req->payload[(i + offset) * 3 + 0];
		leds[i].g = req->payload[(i + offset) * 3 + 1];
		leds[i].b = req->payload[(i + offset) * 3 + 2];
	}

	led_strip::set_pixels(leds, led_board);
}

void led_set(jvs_req_any *req, jvs_resp_any *resp, int led_board)
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

void led_set_fade(jvs_req_any *req, jvs_resp_any *resp, int led_board)
{
	// TODO fade effects

	printf("board: %u \n", led_board);

	if (led_board == 0)
	{
		fade_mode_1 = FADE_MODE_DECREASE;
		fade_value_1 = 255;
	}
	internal_led_set(req, resp, led_board);
}

void led_set_fade_pattern(jvs_req_any *req, jvs_resp_any *resp, int led_board)
{
	printf("Fade pattern \n");
}

void led_disable_response(jvs_req_any *req, jvs_resp_any *resp, int led_board)
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

void led_timeout(const jvs_req_any *req, jvs_resp_any *resp, int led_board)
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

void led_get_board_status(jvs_req_any *req, jvs_resp_any *resp)
{
	resp->len += 4;
	// is this needed??
	*(resp->payload) = 0;
	*(resp->payload + 1) = 0;
	*(resp->payload + 2) = 0;
	*(resp->payload + 3) = 0;
}

void handle_led_command(jvs_req_any *req, jvs_resp_any *resp, int led_board)
{
	resp->src = req->dest;
	resp->dest = req->src;
	resp->cmd = req->cmd;
	resp->len = 3;
	resp->status = 1;
	resp->report = 1;

	printf("CMD %02X \n", req->cmd);

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
		// printf("led payload: ");

		// for (const unsigned char i : req->payload)
		// {
		// 	printf("%02X ", i);
		// }
		// printf("\n");

		led_set(req, resp, led_board);
		break;

	case LED_CMD_SET_LED_FADE:
		// printf("led fade payload: ");

		// for (const unsigned char i : req->payload)
		// {
		// 	printf("%02X ", i);
		// }
		// printf("\n");
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
		printf("Unknown command: 0x%02X\n", req->cmd);
		break;
	}
}

void process_cdc_port(int port, uint8_t *jvs_buff, uint32_t *offset_ptr)
{
	uint32_t offset = *offset_ptr;
	if (const uint32_t max_remaining = MAX_PACKET - offset; max_remaining <= 0)
	{
		offset = 0;
	}

	uint32_t count = tud_cdc_n_read(port, jvs_buff + offset, MAX_PACKET - offset);

	if (count == 0)
	{
		return;
	}

	offset += count;

	HRESULT result = 1;
	jvs_req_any req = {};
	jvs_resp_any resp = {};

	uint8_t out_buffer[MAX_PACKET] = {};
	uint32_t out_len = sizeof(out_buffer);

	result = jvs_process_packet(&req, jvs_buff, offset);

	if (FAILED(result))
	{
		memset(jvs_buff, 0, MAX_PACKET);
		printf("JVS Failed (port %d): HRESULT=0x%08lX\n", port, static_cast<unsigned long>(result));

		result = jvs_write_failure(result, 25, &req, out_buffer, &out_len);
		if (result == S_OK)
		{
			tud_cdc_n_write(port, out_buffer, out_len);
			tud_cdc_n_write_flush(port);
			*offset_ptr = 0;
			return;
		}
		else
		{
			*offset_ptr = 0;
			return;
		}
	}

	handle_led_command(&req, &resp, port);

	if (resp.len != 0)
	{
		result = jvs_write_packet(&resp, out_buffer, &out_len);
	}

	if (SUCCEEDED(result))
	{
		if ((port == 0 && !setting_disable_resp_1) ||
			(port == 1 && !setting_disable_resp_2) ||
			req.cmd == LED_CMD_DISABLE_RESPONSE ||
			req.cmd == LED_CMD_GET_BOARD_INFO ||
			req.cmd == LED_CMD_GET_FIRM_SUM ||
			req.cmd == LED_CMD_GET_PROTOCOL_VER)
		{
			tud_cdc_n_write(port, out_buffer, out_len);
			tud_cdc_n_write_flush(port);
		}

		*offset_ptr = 0;
	}
	else
	{
		*offset_ptr = 0;
	}
}

[[noreturn]] static void core1_loop()
{
	while (true)
	{
		if (fade_mode_1 != 0)
		{
			printf("fading");
			if (fade_timer_1 > 0)
			{
				fade_timer_1 -= DELAY;
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

				printf("fade set");
				led_strip::set_brightness(fade_value_1, 0);
			}
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

		next_frame = time_us_64() + 1000;
		sleep_until(next_frame);
	}
}

void led_test()
{
	led_strip::fill_strip(0);
}

void init()
{
	sleep_ms(50);
	set_sys_clock_khz(150000, true);
	board_init();

	tusb_init();
	stdio_init_all();
}

int main()
{
	init();
	// led_test();
	multicore_launch_core1(core1_loop);
	core0_loop();
}
