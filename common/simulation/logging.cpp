#ifdef ENABLE_LOGGING

#include "common/core/logging.h"

#include <stdio.h>

#include <boost/asio.hpp>

void log_message(const char* format, ...) {
    va_list argp;
    va_start(argp, format);

    auto sigblock = boost::asio::detail::posix_signal_blocker{};
    vprintf(format, argp);

    va_end(argp);
}

#endif
