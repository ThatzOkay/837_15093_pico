#include <string>

#include "hresult.h"
#include "jvs.h"
#include "led.h"
#include "bsp/board_api.h"
#include "device/usbd.h"
#include "hardware/clocks.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "pico/stdio.h"
#include "pico/time.h"

#define MAX_PACKET 255
#define LED_BOARD_01 0
#define LED_BOARD_02 1
#define RGB_ORDER GRB

static uint8_t jvs_buf_1[MAX_PACKET];
static uint32_t offset_1 = 0;
static uint32_t expected_length_1 = 0;

// static uint8_t jvs_buf_2[MAX_PACKET];
// static uint32_t offset_2 = 0;
// static uint32_t expected_length_2 = 0;

static std::string chip_num = "6710";
static std::string board_name = "SEGA";

void led_get_board_info(jvs_req_any *req, jvs_resp_any *resp) {
	resp->len += 18;
	resp->report = 1;
	strcpy(reinterpret_cast<char*>(resp->payload), board_name.c_str());
	*(resp->payload + 8) = 0x0A;
	strcpy(reinterpret_cast<char*>(resp->payload) + 9, chip_num.c_str());
	*(resp->payload + 14) = 0xFF;
	*(resp->payload + 15) = 0xFF;
	*(resp->payload + 16) = 0;
	*(resp->payload + 17) = 204;
}
void led_get_firm_sum(jvs_req_any *req, jvs_resp_any *resp) {
	resp->len += 2;
	*(resp->payload) = 0xFFFF >> 8;
	*(resp->payload + 1) = (uint8_t)0xFFFF;
}

void led_get_protocol_ver(jvs_req_any *req, jvs_resp_any *resp) {
	resp->len += 3;
	*(resp->payload) = 0x01;
	*(resp->payload + 1) = 0x01;
	*(resp->payload + 2) = 0x04;
}

[[noreturn]] static void core1_loop()
{
	while (true)
	{
		if (multicore_fifo_rvalid())
		{
			const uint8_t data = multicore_fifo_pop_blocking();

			multicore_fifo_push_blocking(0x100 | data);
			// (0x100 marks "outgoing to CDC1")
		}
		sleep_ms(1);
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
			offset_1 += count;

			HRESULT result = 0;
			jvs_req_any req = {0};
			jvs_resp_any resp = {0};

			uint8_t out_buffer[MAX_PACKET];
			uint32_t out_len;

			if (offset_1 >= 4 && expected_length_1 == 0)
			{
				expected_length_1 = jvs_buf_1[3] + 4;
			}

			if (expected_length_1 && offset_1 >= expected_length_1)
			{
				result = jvs_process_packet(&req, jvs_buf_1, expected_length_1);

				offset_1 = 0;
				expected_length_1 = 0;
			}

			if (FAILED(result))
			{
				result = jvs_write_failure(result, 25, &req, out_buffer, &out_len);

				if (result == S_OK)
				{
					tud_cdc_n_write(0, out_buffer, out_len);
					tud_cdc_n_write_flush(0);
				}
			}
			else
			{
				if (req.cmd == LED_CMD_GET_BOARD_INFO)
				{
					led_get_board_info(&req, &resp);
				}
				else if (req.cmd == LED_CMD_GET_FIRM_SUM){
					led_get_firm_sum(&req, &resp);
				}
				else if (req.cmd == LED_CMD_GET_PROTOCOL_VER)
				{
					led_get_protocol_ver(&req, &resp);
				}
			}

			if (resp.len != 0) {}
			{
				result = jvs_write_packet(&resp, out_buffer, &out_len);
			}

			if (SUCCEEDED(result))
			{
				tud_cdc_n_write(0, out_buffer, out_len);
				tud_cdc_n_write_flush(0);
			}
		}

		if (tud_cdc_n_available(1))
		{
			uint8_t buf[64];
			const uint32_t count = tud_cdc_n_read(1, buf, sizeof(buf));

			for (uint32_t i = 0; i < count; i++)
			{
				multicore_fifo_push_blocking(buf[i]);
			}
		}

		if (multicore_fifo_rvalid()) {
			uint32_t data = multicore_fifo_pop_blocking();

			if (data & 0x100) {
				uint8_t buffer = data & 0xFF;
				tud_cdc_n_write(1, &buffer, 1);
				tud_cdc_n_write_flush(1);
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
	multicore_launch_core1(core1_loop);
	core0_loop();
}
