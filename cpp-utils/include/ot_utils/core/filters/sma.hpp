#pragma once

#include <limits>

#include "ot_utils/core/fixed_point.hpp"

// Simple Moving Average Algorithm
// A finite impulse response (FIR) or 'windowed' filter

// Use this filter when you have a finite window
// of data that is particularly noisy.
namespace ot_utils {

namespace filters {

template <class IntegerValue, int WindowSize = 10, int Radix = 8>
class SimpleMovingAverage {
    using I = IntegerValue;

  public:
    auto compute(I input) -> I {
        // save previous value to determine
        // if the signal is stable.
        previous = running_average;

        bool return_input = bool(current_index < WindowSize - 1);

        auto compute_average = fixed_point::fixed_point_multiply<I>(
            window_divisor, (input - window[current_index]), Radix);
        running_average = running_average + compute_average;

        window[current_index] = input;
        current_index += 1;

        if (return_input) {
            // average signal is not ready yet.
            return input;
        }

        if (current_index >= WindowSize) {
            current_index = 0;
            exceeded_window = true;
        }

        return running_average;
    }

    auto stop_filter() -> bool {
        if (exceeded_window) {
            // difference is anything up to 1
            return !bool((running_average - previous) & WHOLE_NUMBER_MASK);
        }
        return false;
    }

  private:
    I previous = 0;
    I running_average = 0;
    std::array<I, WindowSize> window{0};
    int current_index = 0;
    bool exceeded_window = false;

    const I window_divisor = fixed_point::convert_to_fixed_point<I>(
        static_cast<float>(1.0 / WindowSize), Radix);

    const I WHOLE_NUMBER_MASK = 0xFF << Radix;

};
}  // namespace filters

}  // namespace ot_utils
