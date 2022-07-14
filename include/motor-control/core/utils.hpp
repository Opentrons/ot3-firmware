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

auto fixed_point_multiply(sq31_31 a, uint32_t b) -> uint32_t;

// the radix_offset_0 struct and its presence in the fixed_point_multiply
// overload for sq31_31/int32 is required to differentiate it from
// sq31_31/sq0_31, since 0_31 is a typedef for int32, and the compiler
// considers it the same when it comes to function overloading.
struct radix_offset_0 {};

auto fixed_point_multiply(sq31_31 a, int32_t b, radix_offset_0) -> int32_t;

auto fixed_point_to_float(uint32_t data, int to_radix) -> float;

auto signed_fixed_point_to_float(int32_t data, int to_radix) -> float;
