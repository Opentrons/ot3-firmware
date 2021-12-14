#pragma once

#include <array>
#include <concepts>
#include <cstdint>

namespace adc {

constexpr const size_t BufferSize = 5;

template <class ADC>
concept has_get_reading = requires(ADC adc_comms) {
    {adc_comms.get_readings()};
};

}  // namespace spi