#pragma once

#include <cstdint>

using sq0_31 = int32_t;  // 0: signed bit,  1-31: fractional bits
using sq14_15 = int32_t;  // 0: signed bit,  1-31: fractional bits
using q31_31 =
    uint64_t;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

using sq31_31 = int64_t;

using ticks = uint64_t;
using steps_per_tick = sq0_31;
using steps_per_tick_sq = sq0_31;