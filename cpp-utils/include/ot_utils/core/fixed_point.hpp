#pragma once

#include <cstdint>
#include <limits>
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

constexpr int UINT16_T_MAX = std::numeric_limits<uint16_t>::max();
constexpr int INT16_T_MAX = std::numeric_limits<int16_t>::max();
constexpr int UINT32_T_MAX = std::numeric_limits<uint32_t>::max();
constexpr int INT32_T_MAX = std::numeric_limits<int32_t>::max();
constexpr double UINT64_T_MAX = std::numeric_limits<uint64_t>::max();
constexpr double INT64_T_MAX = std::numeric_limits<int64_t>::max();

template <class integer_t>
auto convert_to_fixed_point(double value, int to_radix)
    -> std::enable_if_t<std::numeric_limits<integer_t>::max() == UINT16_T_MAX,
                        uint16_t>;

template <class integer_t>
auto convert_to_fixed_point(double value, int to_radix)
    -> std::enable_if_t<std::numeric_limits<integer_t>::max() == INT16_T_MAX,
                        int16_t>;

template <class integer_t>
auto convert_to_fixed_point(double value, int to_radix)
    -> std::enable_if_t<std::numeric_limits<integer_t>::max() == UINT32_T_MAX,
                        uint32_t>;

template <class integer_t>
auto convert_to_fixed_point(double value, int to_radix)
    -> std::enable_if_t<std::numeric_limits<integer_t>::max() == INT32_T_MAX,
                        int32_t>;

template <class integer_t>
auto convert_to_fixed_point(double value, int to_radix)
    -> std::enable_if_t<std::numeric_limits<integer_t>::max() == UINT64_T_MAX,
                        uint64_t>;

template <class integer_t>
auto convert_to_fixed_point(double value, int to_radix)
    -> std::enable_if_t<std::numeric_limits<integer_t>::max() == INT64_T_MAX,
                        int64_t>;

template <class integer_t_c>
auto fixed_point_multiply(std::size_t a, std::size_t b, int radix)
    -> std::enable_if_t<std::numeric_limits<integer_t_c>::max() == UINT16_T_MAX,
                        uint16_t>;

template <class integer_t_c>
auto fixed_point_multiply(std::size_t a, std::size_t b, int radix)
    -> std::enable_if_t<std::numeric_limits<integer_t_c>::max() == INT16_T_MAX,
                        int16_t>;

template <class integer_t_c>
auto fixed_point_multiply(std::size_t a, std::size_t b, int radix)
    -> std::enable_if_t<std::numeric_limits<integer_t_c>::max() == UINT32_T_MAX,
                        uint32_t>;

template <class integer_t_c>
auto fixed_point_multiply(std::size_t a, std::size_t b, int radix)
    -> std::enable_if_t<std::numeric_limits<integer_t_c>::max() == INT32_T_MAX,
                        int32_t>;

}  // namespace fixed_point
}  // namespace ot_utils
