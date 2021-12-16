#pragma once

#include <array>
#include <concepts>
#include <cstdint>

#include "common/firmware/adc_comms.hpp"

#include "platform_specific_hal_conf.h"
#pragma GCC diagnostic push
// NOLINTNEXTLINE(clang-diagnostic-unknown-warning-option)
#pragma GCC diagnostic ignored "-Wvolatile"
#include "common/firmware/adc.h"
#pragma GCC diagnostic pop

namespace adc {

constexpr const size_t BufferSize = 5;

template <class ADC>
concept has_get_reading = requires(ADC adc_comms, ADC_interface ADC_intf_instance) {
    {adc_comms.get_readings(ADC_intf_instance)};
};

}  // namespace spi