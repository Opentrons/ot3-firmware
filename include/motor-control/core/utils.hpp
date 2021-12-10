#pragma once

#include "motor-control/core/types.hpp"
/*
 * Fixed point math helpers. For now, you should only pass in two numbers with
 * the same radix position. In the future, we can expand on these helper
 * functions to account for different radix positions.
 */

sq0_31 convert_to_fixed_point(float value, int to_radix) {
    return static_cast<int32_t>(value * static_cast<float>(1LL << to_radix));
}

sq0_31 fixed_point_multiply(sq0_31 a, sq0_31 b) {
    int64_t resultant = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<sq0_31>((resultant >> 31) & 0xFFFFFFFF);
}
