#pragma once

#include <cstdint>

#include "rear-panel/core/lights/animation_queue.hpp"

namespace lights::math {

auto calculate_power(lights::Transition transition, double start_power,
                     double end_power, uint32_t ms_count, uint32_t ms_total)
    -> double;

auto color_interpolate(const lights::Color& first, const lights::Color& second,
                       lights::Transition transition, uint32_t ms_count,
                       uint32_t ms_total) -> lights::Color;

}  // namespace lights::math