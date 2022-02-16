#pragma once

#include <concepts>
#include <cstdint>

#include "common/core/adc_channel.hpp"

namespace adc {

struct MillivoltsReadings {
    millivolts_t z_motor;
    millivolts_t a_motor;
    millivolts_t gripper;
};

class BaseADC {
  public:
    static constexpr uint32_t ADC_FULLSCALE_MV = 3300;
    static constexpr uint32_t ADC_FULLSCALE_READING = 4095;
    using ChannelType = BaseADCChannel<ADC_FULLSCALE_MV, ADC_FULLSCALE_READING>;
    BaseADC() = default;
    BaseADC(const BaseADC&) = default;
    auto operator=(const BaseADC&) -> BaseADC& = default;
    BaseADC(BaseADC&&) noexcept = default;
    auto operator=(BaseADC&&) noexcept -> BaseADC& = default;
    virtual ~BaseADC() = default;

    auto get_voltages() -> MillivoltsReadings {
        return MillivoltsReadings{
            .z_motor = get_z_channel().get_voltage(),
            .a_motor = get_a_channel().get_voltage(),
            .gripper = get_gripper_channel().get_voltage()};
    }

  protected:
    virtual auto get_z_channel() -> ChannelType& = 0;
    virtual auto get_a_channel() -> ChannelType& = 0;
    virtual auto get_gripper_channel() -> ChannelType& = 0;
};

}  // namespace adc
