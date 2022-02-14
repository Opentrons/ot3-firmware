#ifdef ENABLE_LOGGING

#include <stdio.h>
#include "common/core/logging.h"

void log_message(const char * format, ...) {
    va_list argp;
    va_start(argp, format);

    vprintf(format, argp);

    va_end(argp);
}

#endif
