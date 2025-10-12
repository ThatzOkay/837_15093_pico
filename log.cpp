#include "log.h"
#include <cstdarg>
#include <cstdio>
#include "config.h"

void log(const char *format, ...)
{
#if ENABLE_DEBUG
    uint8_t out_buffer[MAX_PACKET] = {};

    va_list args;
    va_start(args, format);
    int len = vsnprintf((char*)out_buffer, sizeof(out_buffer), format, args);
    va_end(args);

    if (len > 0)
    {
        tud_cdc_n_write(2, out_buffer, (len < sizeof(out_buffer)) ? len : sizeof(out_buffer));
        tud_cdc_n_write_flush(2);
    }
#endif
}