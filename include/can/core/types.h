#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

enum CanFilterConfig {
    reject,
    to_fifo0,
    to_fifo1,
    to_fifo0_high_priority,
    to_fifo1_high_priority,
    };

enum CanFilterType {
    /* if the arbitration id matches either of the two values. */
    exact,
    /* if arbitration id is within the range of first and second arguments. */
    range,
    /* if first argument equals the arbitration id masked by second argument. */
    mask,
    };

/**
 * Can FD message length.
 */
enum CanFDMessageLength {
    l0 = 0,
    l1 = 1,
    l2 = 2,
    l3 = 3,
    l4 = 4,
    l5 = 5,
    l6 = 6,
    l7 = 7,
    l8 = 8,
    l12 = 12,
    l16 = 16,
    l20 = 20,
    l24 = 24,
    l32 = 32,
    l48 = 48,
    l64 = 64,
    };

/**
 * Round length to the nearest CanFDMessageLength
 * @param length a value between 0-64.
 * @return rounded length
 */
enum CanFDMessageLength to_canfd_length(uint32_t length);


#ifdef __cplusplus
}  // extern "C"
#endif  // __cplusplus

