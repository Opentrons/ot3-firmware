#pragma once

#include <cstdint>

using sq0_31 = int32_t;  // 0: signed bit,  1-31: fractional bits
using sq15_16 =
    int32_t;  // 0: signed bit,  1-14: integer bits, 15-31: fractional bits
using q31_31 =
    uint64_t;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

using sq31_31 = int64_t;

using stepper_timer_ticks = uint64_t;
using brushed_timer_ticks = uint64_t;
using steps_per_tick = sq0_31;
using steps_per_tick_sq = sq0_31;