#include "log.h"
#include <cstdarg>
#include <cstdio>

void log(const char *format, ...)
{
#if DEBUG_LOG == 1
    return;
#else
    // #if ENABLE_UART == 1
    // // TODO uart log maybe through tiny usb cdc 0
    // #else
    va_list argptr;
    va_start(argptr, format);
    vprintf(format, argptr);
    va_end(argptr);
// #endif
#endif
}