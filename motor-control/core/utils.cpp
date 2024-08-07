#include "motor-control/core/utils.hpp"

#include "motor-control/core/types.hpp"

/*
 * Fixed point math helpers. For now, you should only pass in two numbers with
 * the same radix position. In the future, we can expand on these helper
 * functions to account for different radix positions.
 */

sq0_31 convert_to_fixed_point(float value, int to_radix) {
    return static_cast<int32_t>(value * static_cast<float>(1LL << to_radix));
}

sq31_31 convert_to_fixed_point_64_bit(float value, int to_radix) {
    return static_cast<int64_t>(value * static_cast<float>(1LL << to_radix));
}

sq0_31 fixed_point_multiply(sq0_31 a, sq0_31 b) {
    int64_t result = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<sq0_31>((result >> 31) & 0xFFFFFFFF);
}

sq0_31 fixed_point_multiply(sq31_31 a, sq0_31 b) {
    int64_t result = a * static_cast<int64_t>(b);
    return static_cast<sq0_31>((result >> 31) & 0xFFFFFFFF);
}

float fixed_point_to_float(uint32_t data, int to_radix) {
    return (1.0 * static_cast<float>(data)) / static_cast<float>(1 << to_radix);
}

float signed_fixed_point_to_float(int32_t data, int to_radix) {
    return (1.0 * static_cast<float>(data)) / static_cast<float>(1 << to_radix);
}

uint32_t fixed_point_multiply(sq31_31 a, uint32_t b) {
    uint64_t result = a * static_cast<uint64_t>(b);
    return static_cast<uint32_t>((result >> 31) & 0xFFFFFFFF);
}

int32_t fixed_point_multiply(sq31_31 a, int32_t b, radix_offset_0 _) {
    static_cast<void>(_);
    int64_t result = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>(result >> 31);
}
