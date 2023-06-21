#pragma once

#include <cstdint>
#include <cstdlib>

#include <type_traits>

namespace ot_utils {
namespace fixed_point {

using sq0_31 = int32_t;  // 0: signed bit,  1-31: fractional bits
using q31_31 =
    uint64_t;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

using sq31_31 = int64_t;

/*
 * Fixed point math helpers. For now, you should only pass in two numbers with
 * the same radix position. In the future, we can expand on these helper
 * functions to account for different radix positions.
 */


template <typename integer_t>
requires std::is_integral_v<integer_t>
auto convert_to_fixed_point(double value, int to_radix) -> integer_t {
    return integer_t(value * double(1LL << to_radix)); 
}

template <typename integer_t>
struct size_up;

template <>
struct size_up<uint16_t> {
    using type = uint32_t;
};

template <>
struct size_up<uint32_t> {
    using type = uint64_t;
};


template <>
struct size_up<int16_t> {
    using type = int32_t;
};

template <>
struct size_up<int32_t> {
    using type = int64_t;
};


template <typename integer_t>
requires std::is_integral_v<integer_t>
auto fixed_point_multiply(std::size_t a, std::size_t b, int radix)
    -> integer_t {
    using SizeUpType = size_up<integer_t>::type;
    SizeUpType result = static_cast<SizeUpType>(a) * static_cast<SizeUpType>(b);
    return integer_t((result >> radix) & ~(1<<sizeof(integer_t)));
}

}  // namespace fixed_point
}  // namespace ot_utils
