#ifdef ENABLE_LOGGING

#include <stdio.h>
#include "common/core/logging.h"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/synchronization.hpp"

static auto mutex = freertos_synchronization::FreeRTOSMutex{};


/** A message */
void log_message(const char * format, ...) {

    // See "Known Issues" section:
    //  https://www.freertos.org/FreeRTOS-simulator-for-Linux.html
    auto lock = synchronization::Lock{mutex};

    va_list argp;
    va_start(argp, format);

    vprintf(format, argp);

    va_end(argp);
}

#endif
