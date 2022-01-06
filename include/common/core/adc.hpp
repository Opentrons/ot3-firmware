#pragma once

#include <concepts>
#include <stdint.h>

namespace adc {
/*
template <class ADC>
concept has_get_reading = requires(ADC adc_comms) {
    {adc_comms.get_readings()};
};
*/
struct voltage_read {
    uint32_t z_motor;
    uint32_t a_motor;
    uint32_t gripper;
};

class BaseADC{
    public:
        BaseADC() = default;
        virtual ~BaseADC() = default;
        BaseADC(const BaseADC&) = default;
        auto operator=(const BaseADC&) -> BaseADC& = default;
        BaseADC(BaseADC&&) = default;
        auto operator=(BaseADC&&) -> BaseADC& = default;
        virtual auto get_readings() -> voltage_read =0;
};

}  // namespace adc