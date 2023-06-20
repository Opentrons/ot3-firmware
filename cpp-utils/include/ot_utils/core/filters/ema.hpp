#pragma once

#include "ot_utils/core/fixed_point.hpp"

// Exponential Moving Average Algorithm
// A infinite impulse response (IIR) or non-windowed
namespace ot_utils {

namespace filters {

template <typename IntegerValue, int AlphaDivisor, int Radix>
class ExponentialMovingAverage {
  public:
    auto compute(IntegerValue data) -> IntegerValue {
        IntegerValue current_weighted =
            fixed_point::fixed_point_multiply<IntegerValue>(
                fixed_point_current_weight, data, Radix);
        IntegerValue prior_weighted =
            fixed_point::fixed_point_multiply<IntegerValue>(
                prior_average, fixed_point_prior_weight, Radix);

        IntegerValue filtered_data = current_weighted + prior_weighted;
        prior_average = filtered_data;
        return filtered_data;
    }

  private:
    IntegerValue prior_average = 0;
    float current_weight = static_cast<float>(1.0 / AlphaDivisor);
    float prior_weight = 1 - current_weight;
    IntegerValue fixed_point_current_weight =
        fixed_point::convert_to_fixed_point<IntegerValue>(current_weight,
                                                          Radix);
    IntegerValue fixed_point_prior_weight =
        fixed_point::convert_to_fixed_point<IntegerValue>(prior_weight, Radix);
};
}  // namespace filters
}  // namespace ot_utils
