#include "log.h"
#include <cstdarg>
#include <cstdio>

#include "config.h"
#include "pico/stdio.h"
#include "tusb.h"

void debug_log(const char* format, ...)
{
    if (led_cfg->debug.enable)
    {
        char out_buffer[256] = {};

        va_list args;
        va_start(args, format);
        const int len = vsnprintf(out_buffer, sizeof(out_buffer), format, args);
        va_end(args);

        if (len > 0)
        {
            tud_cdc_n_write(2, out_buffer, len < sizeof(out_buffer) ? len : sizeof(out_buffer));
            tud_cdc_n_write_flush(2);
        }
    }
}

void cli_log(const char* format, ...)
{
    char out_buffer[MAX_PACKET] = {};

    va_list args;
    va_start(args, format);
    int len = vsnprintf(out_buffer, sizeof(out_buffer), format, args);
    va_end(args);

    if (len <= 0) return;

    int written = 0;
    while (written < len)
    {
        int avail = tud_cdc_n_write_available(2);
        if (avail > 0)
        {
            int to_write = (len - written < avail) ? (len - written) : avail;
            tud_cdc_n_write(2, &out_buffer[written], to_write);
            tud_cdc_n_write_flush(2);
            written += to_write;
        }
        tud_task();
    }
}
