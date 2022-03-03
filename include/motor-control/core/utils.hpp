#pragma once

#include "motor-control/core/types.hpp"
/*
 * Fixed point math helpers. For now, you should only pass in two numbers with
 * the same radix position. In the future, we can expand on these helper
 * functions to account for different radix positions.
 */

auto convert_to_fixed_point(float value, int to_radix) -> sq0_31;

auto convert_to_fixed_point_64_bit(float value, int to_radix) -> sq31_31;

auto fixed_point_multiply(sq0_31 a, sq0_31 b) -> sq0_31;

auto fixed_point_multiply(sq31_31 a, sq0_31 b) -> sq0_31;

auto fixed_point_to_float(uint32_t data, int to_radix) -> float;
