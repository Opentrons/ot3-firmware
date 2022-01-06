#pragma once

#include <concepts>

namespace adc {

template <class ADC>
concept has_get_reading = requires(ADC adc_comms) {
    {adc_comms.get_readings()};
};

}  // namespace adc