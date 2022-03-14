#pragma once

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#ifdef ENABLE_LOGGING

#include <stdarg.h>

/**
 * Log message function. Not to be used directly. Use LOG macro.
 */
void log_message(const char * format, ...);

#define LOG(format, ...) log_message(format, ## __VA_ARGS__)

#else

#define LOG(...)

#endif


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus
