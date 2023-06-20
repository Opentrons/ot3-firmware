#include "ot_utils/core/fixed_point.hpp"

/*
 * Fixed point math helpers. For now, you should only pass in two numbers with
 * the same radix position. In the future, we can expand on these helper
 * functions to account for different radix positions.
 */

static constexpr int64_t multiply_mask_64 = 0xFFFFFFFF;
static constexpr int32_t multiply_mask_32 = 0xFFFF;
static constexpr int multiply_shift = 31;

template <>
auto ot_utils::fixed_point::convert_to_fixed_point<uint16_t>(double value,
                                                             int to_radix)
    -> uint16_t {
    return static_cast<uint16_t>(value * static_cast<double>(1LL << to_radix));
}

template <>
auto ot_utils::fixed_point::convert_to_fixed_point<int16_t>(double value,
                                                            int to_radix)
    -> int16_t {
    return static_cast<uint16_t>(value * static_cast<double>(1LL << to_radix));
}

template <>
auto ot_utils::fixed_point::convert_to_fixed_point<uint32_t>(double value,
                                                             int to_radix)
    -> uint32_t {
    return static_cast<uint32_t>(value * static_cast<double>(1LL << to_radix));
}

template <>
auto ot_utils::fixed_point::convert_to_fixed_point<int32_t>(double value,
                                                            int to_radix)
    -> int32_t {
    return static_cast<int32_t>(value * static_cast<double>(1LL << to_radix));
}

template <>
auto ot_utils::fixed_point::convert_to_fixed_point<uint64_t>(double value,
                                                             int to_radix)
    -> uint64_t {
    return static_cast<uint32_t>(value * static_cast<double>(1LL << to_radix));
}

template <>
auto ot_utils::fixed_point::convert_to_fixed_point<int64_t>(double value,
                                                            int to_radix)
    -> int64_t {
    return static_cast<int64_t>(value * static_cast<double>(1LL << to_radix));
}

template <>
auto ot_utils::fixed_point::fixed_point_multiply<uint16_t>(std::size_t a,
                                                           std::size_t b,
                                                           int radix)
    -> uint16_t {
    uint32_t result = static_cast<uint32_t>(a) * static_cast<uint32_t>(b);
    return static_cast<uint16_t>((result >> radix) & multiply_mask_32);
}

template <>
auto ot_utils::fixed_point::fixed_point_multiply<int16_t>(std::size_t a,
                                                          std::size_t b,
                                                          int radix)
    -> int16_t {
    int32_t result = static_cast<int32_t>(a) * static_cast<int32_t>(b);
    return static_cast<int16_t>((result >> radix) & multiply_mask_32);
    ;
}

template <>
auto ot_utils::fixed_point::fixed_point_multiply<uint32_t>(std::size_t a,
                                                           std::size_t b,
                                                           int radix)
    -> uint32_t {
    uint64_t result = static_cast<uint64_t>(a) * static_cast<uint64_t>(b);
    return static_cast<uint32_t>((result >> radix) & multiply_mask_64);
}

template <>
auto ot_utils::fixed_point::fixed_point_multiply<int32_t>(std::size_t a,
                                                          std::size_t b,
                                                          int radix)
    -> int32_t {
    int64_t result = static_cast<int64_t>(a) * static_cast<int64_t>(b);
    return static_cast<int32_t>((result >> radix) & multiply_mask_64);
}
