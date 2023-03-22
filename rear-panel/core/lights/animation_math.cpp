#include "rear-panel/core/lights/animation_math.hpp"

#include <cmath>

#include "rear-panel/core/lights/animation_queue.hpp"

static constexpr double PI = std::acos(-1);

static auto calculate_linear_power(double start_power, double end_power,
                                   uint32_t ms_count, uint32_t ms_total)
    -> double {
    // Simple linear regression. At count of 0ms we return start_power, and at
    // the end we return the end_power.
    double slope = end_power - start_power;
    double scale =
        static_cast<double>(ms_count) / static_cast<double>(ms_total);
    return (scale * slope) + start_power;
}

static auto calculate_sin_power(double start_power, double end_power,
                                uint32_t ms_count, uint32_t ms_total)
    -> double {
    double slope = end_power - start_power;
    // Percent done (count / total)
    double pct = static_cast<double>(ms_count) / static_cast<double>(ms_total);
    // Scale to rads. 0% should be π rads, 100% should be 2π
    double rads = PI + (pct * PI);
    // We want to scale the cosine of the angle in the range [0,1], so we add
    // 1 to the cos and then divide the result by 2
    double scale = (1.0 + std::cos(rads)) / 2.0F;
    return (scale * slope) + start_power;
}

auto lights::math::calculate_power(lights::Transition transition,
                                   double start_power, double end_power,
                                   uint32_t ms_count, uint32_t ms_total)
    -> double {
    if (ms_total == 0) {
        transition = lights::Transition::instant;
    }
    switch (transition) {
        case lights::Transition::linear:
            return calculate_linear_power(start_power, end_power, ms_count,
                                          ms_total);
            break;
        case lights::Transition::sinusoid:
            return calculate_sin_power(start_power, end_power, ms_count,
                                       ms_total);
            break;
        case lights::Transition::instant:
            return end_power;
            break;
        default:
            return end_power;
    }
}