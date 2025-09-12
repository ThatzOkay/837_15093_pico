#include <string>

#include "hresult.h"
#include "jvs.h"
#include "led.h"
#include "bsp/board_api.h"
#include "device/usbd.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "pico/stdio.h"
#include "pico/time.h"

#define MAX_PACKET 255
#define LED_BOARD_01 0
#define LED_BOARD_02 1
#define RGB_ORDER GRB

static uint8_t settings_timeout1;
static bool setting_disable_resp_1 = false;
static uint8_t jvs_buf_1[MAX_PACKET];
static uint32_t offset_1 = 0;
static uint32_t expected_length_1 = 0;

static uint8_t settings_timeout2;
static bool setting_disable_resp_2 = false;
static uint8_t jvs_buf_2[MAX_PACKET];
static uint32_t offset_2 = 0;
static uint32_t expected_length_2 = 0;

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
	}
	else
	{
		setting_disable_resp_2 = false;
	}
	// setting_led_count = JVS_MAX_LEDS;
	// fade_mode = FADE_MODE_NONE;
	// fade_timer = FADE_TIMER_DEFAULT;
	// fade_timer_max = FADE_TIMER_DEFAULT;
	// fade_modifier = 1;
	// fade_value = 255;

	// wipe_leds(led_board);
}

void led_set(jvs_req_any *req, jvs_resp_any *resp, int led_board)
{
}

void led_set_fade(jvs_req_any *req, jvs_resp_any *resp, int led_board)
{
}

void led_set_fade_pattern(jvs_req_any *req, jvs_resp_any *resp, int led_board)
{
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

void led_timeout(jvs_req_any *req, jvs_resp_any *resp, int led_board)
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
	printf("LED 1 CMD: 0x%02X \n", req->cmd);

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
		led_disable_response(req, resp, led_board );
		break;

	case LED_CMD_RESET:
		led_reset(req, resp, led_board);
		break;
	case LED_CMD_SET_LED:
		printf("led payload: ");

		for (const unsigned char i : req->payload)
		{
			printf("%02X ", i);
		}
		printf("\n");

		led_set(req, resp, 0);
		break;

	case LED_CMD_SET_LED_FADE:
		led_set_fade(req, resp, 0);
		break;

	case LED_CMD_SET_LED_FADE_PATTERN:
		led_set_fade_pattern(req, resp, 0);
		break;

	case LED_CMD_TIMEOUT:
		led_timeout(req, resp, 0);
		break;

	case LED_CMD_GET_BOARD_STATUS:
		led_get_board_status(req, resp);
		break;

	default:
		printf("Unknown command: 0x%02X\n", req->cmd);
		break;
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
			uint32_t count = tud_cdc_n_read(0, jvs_buf_1 + offset_1, MAX_PACKET - offset_1);

			if (count == 0 || is_all_zero(jvs_buf_1, 4))
			{
				continue;
			}

			offset_1 += count;

			HRESULT result = 1;
			jvs_req_any req = {0};
			jvs_resp_any resp = {0};

			uint8_t out_buffer[MAX_PACKET] = {0};
			uint32_t out_len = 255;

			// if (offset_1 >= 4 && expected_length_1 == 0)
			// {
			// 	expected_length_1 = jvs_buf_1[3] + 4;
			// }

			// if (expected_length_1 && offset_1 >= expected_length_1)
			// {
			result = jvs_process_packet(&req, jvs_buf_1, count);

			offset_1 = 0;
			expected_length_1 = 0;
			// }
			// else
			// {
			// 	printf("expected 1: %u \n", expected_length_1);
			// 	printf("offset 1: %u \n", offset_1);
			// 	printf("Count: %u \n", count);
			// 	printf("Raw tud read buff:");
			// 	for (int i = 0; i < sizeof(jvs_buf_1); i++)
			// 	{
			// 		printf("%02X ", jvs_buf_1[i]);
			// 	}
			// 	printf("\n");
			// 	continue;
			// }

			if (FAILED(result))
			{
				memset(jvs_buf_1, 0, sizeof(jvs_buf_1));
				printf("JVS Failed 1 \n");
				result = jvs_write_failure(result, 25, &req, out_buffer, &out_len);

				if (result == S_OK)
				{
					tud_cdc_n_write(0, out_buffer, out_len);
					tud_cdc_n_write_flush(0);
				}
			}
			else
			{
				handle_led_command(&req, &resp, 0);
			}

			if (resp.len != 0)
			{
				result = jvs_write_packet(&resp, out_buffer, &out_len);
			}

			if (SUCCEEDED(result))
			{
				if (!setting_disable_resp_1 || req.cmd == LED_CMD_DISABLE_RESPONSE || req.cmd == LED_CMD_GET_BOARD_INFO || req.cmd == LED_CMD_GET_FIRM_SUM || req.cmd == LED_CMD_GET_PROTOCOL_VER)
				{
					printf("Response length 1: %d\nPayload: ", out_len);
					for (int i = 0; i < out_len; i++)
					{
						printf("%02X ", out_buffer[i]);
					}
					printf("\n");
					tud_cdc_n_write(0, out_buffer, out_len);
					tud_cdc_n_write_flush(0);
				}
			}
			else
			{
				printf("Error writing packet %ld \n", result);
			}
		}

		if (tud_cdc_n_available(1))
		{
			uint32_t count = tud_cdc_n_read(1, jvs_buf_2 + offset_2, MAX_PACKET - offset_2);

			if (count == 0 || is_all_zero(jvs_buf_2, 4))
			{
				continue;
			}

			offset_2 += count;

			HRESULT result = 1;
			jvs_req_any req = {0};
			jvs_resp_any resp = {0};

			uint8_t out_buffer[MAX_PACKET] = {0};
			uint32_t out_len = 255;

			// if (offset_2 >= 4 && expected_length_2 == 0)
			// {
			// 	expected_length_2 = jvs_buf_2[3] + 4;
			// }

			// if (expected_length_2 && offset_2 >= expected_length_2)
			// {
			result = jvs_process_packet(&req, jvs_buf_2, expected_length_2);

			offset_2 = 0;
			expected_length_2 = 0;
			// }
			// else
			// {
			// 	printf("expected 2: %u \n", expected_length_2);
			// 	printf("offset 2: %u \n", offset_2);
			// 	printf("Count: %u \n", count);
			// 	printf("Raw tud read buff:");
			// 	for (int i = 0; i < sizeof(jvs_buf_2); i++)
			// 	{
			// 		printf("%02X ", jvs_buf_2[i]);
			// 	}
			// 	printf("\n");
			// 	continue;
			// }

			if (FAILED(result))
			{
				printf("JVS Failed 2 \n");
				result = jvs_write_failure(result, 25, &req, out_buffer, &out_len);

				if (result == S_OK)
				{
					tud_cdc_n_write(1, out_buffer, out_len);
					tud_cdc_n_write_flush(1);
				}
			}
			else
			{
				handle_led_command(&req, &resp, 0);
			}

			if (resp.len != 0)
			{
				result = jvs_write_packet(&resp, out_buffer, &out_len);
			}

			if (SUCCEEDED(result))
			{
				if (!setting_disable_resp_1 || req.cmd == LED_CMD_DISABLE_RESPONSE || req.cmd == LED_CMD_GET_BOARD_INFO || req.cmd == LED_CMD_GET_FIRM_SUM || req.cmd == LED_CMD_GET_PROTOCOL_VER)
				{

					printf("Response length 2: %d\nPayload: ", out_len);
					for (int i = 0; i < out_len; i++)
					{
						printf("%02X ", out_buffer[i]);
					}
					printf("\n");
					tud_cdc_n_write(1, out_buffer, out_len);
					tud_cdc_n_write_flush(1);
				}
			}
			else
			{
				printf("Error writing packet %ld \n", result);
			}
		}

		sleep_until(next_frame);
		next_frame += 1000;
	}
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
	core0_loop();
}
