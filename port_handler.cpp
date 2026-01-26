#include "port_handler.h"
#include "config.h"
#include "hresult.h"
#include "jvs.h"
#include "led_commands.h"
#include "log.h"
#include "pico/time.h"
#include "tusb.h"

static bool wait_for_readable(uart_inst_t* port, uint32_t timeout_us)
{
    absolute_time_t start = get_absolute_time();
    while (!uart_is_readable(port))
    {
        if (absolute_time_diff_us(start, get_absolute_time()) > timeout_us)
            return false;
    }
    return true;
}

void process_cdc_port(int port, uint8_t* jvs_buff, uint32_t* offset_ptr)
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
        debug_log("JVS Failed (port %d): HRESULT=0x%08lX\n", port, static_cast<unsigned long>(result));

        result = jvs_write_failure(result, 25, &req, out_buffer, &out_len);
        if (result == S_OK)
        {
            tud_cdc_n_write(port, out_buffer, out_len);
            tud_cdc_n_write_flush(port);
            *offset_ptr = 0;
            return;
        }
        *offset_ptr = 0;
        return;
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

bool read_jvs_packet(uart_inst_t* port, uint8_t* buf, uint32_t* len_out)
{
    int index = 0;

    if (!uart_is_readable(port))
        return false;

    absolute_time_t start = get_absolute_time();
    while (true)
    {
        if (uart_is_readable(port))
        {
            uint8_t ch = uart_getc(port);
            if (ch == 0xE0)
            {
                buf[index++] = ch;
                break;
            }
        }

        if (absolute_time_diff_us(start, get_absolute_time()) > UART_READ_TIMEOUT_US)
        {
            //debug_log("UART Timeout reached \n");
            return false;
        }
    }

    if (!wait_for_readable(port, UART_READ_TIMEOUT_US))
        return false;

    // log("Sync byte found \n");

    uint8_t src = uart_getc(port);
    uint8_t dest = uart_getc(port);
    uint8_t length = uart_getc(port);

    buf[index++] = src;
    buf[index++] = dest;
    buf[index++] = length;
    for (int i = 0; i < length + 1; i++)
    {
        if (!wait_for_readable(port, UART_READ_TIMEOUT_US))
            return false;
        buf[index++] = uart_getc(port);
    }

    *len_out = index;

    // log("Raw read buff:");
    // for (int i = 0; i < index; i++)
    // 	log(" %02X", buf[i]);
    // log("\n");

    return true;
}

void process_uart_port(uart_inst_t* port, uint8_t* jvs_buff, uint32_t* offset_ptr)
{
    uint32_t len = 0;
    if (!read_jvs_packet(port, jvs_buff, &len))
        return;

    HRESULT result = 1;
    jvs_req_any req = {};
    jvs_resp_any resp = {};

    uint8_t out_buffer[MAX_PACKET] = {};
    uint32_t out_len = sizeof(out_buffer);

    result = jvs_process_packet(&req, jvs_buff, len);

    if (FAILED(result))
    {
        memset(jvs_buff, 0, MAX_PACKET);
        debug_log("JVS Failed (port %d): HRESULT=0x%08lX\n", port, static_cast<unsigned long>(result));

        result = jvs_write_failure(result, 25, &req, out_buffer, &out_len);
        if (result == S_OK)
        {
            for (uint8_t i = 0; i < out_len; i++)
            {
                uart_putc_raw(port, out_buffer[i]);
            }
        }
        return;
    }

    if (port == UART0_ID)
        handle_led_command(&req, &resp, 0);
    else
        handle_led_command(&req, &resp, 1);

    if (resp.len != 0)
        result = jvs_write_packet(&resp, out_buffer, &out_len);

    if (SUCCEEDED(result))
    {
        if ((port == UART0_ID && !setting_disable_resp_1) ||
            (port == UART1_ID && !setting_disable_resp_2) ||
            req.cmd == LED_CMD_DISABLE_RESPONSE ||
            req.cmd == LED_CMD_GET_BOARD_INFO ||
            req.cmd == LED_CMD_GET_FIRM_SUM ||
            req.cmd == LED_CMD_GET_PROTOCOL_VER)
        {
            for (uint8_t i = 0; i < out_len; i++)
            {
                uart_putc_raw(port, out_buffer[i]);
            }
        }
    }
}