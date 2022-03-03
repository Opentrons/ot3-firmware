#pragma once

#include "common/tests/mock_adc_channel.hpp"
#include "head/core/adc.hpp"

namespace adc {

class MockADC : public adc::BaseADC {
    using MockChannelType = MockADCChannel<BaseADC::ADC_FULLSCALE_MV,
                                           BaseADC::ADC_FULLSCALE_READING>;

  public:
    MockADC() = default;
    MockADC(uint16_t z_reading, uint16_t a_reading, uint16_t gripper_reading)
        : z_channel(z_reading),
          a_channel(a_reading),
          gripper_channel(gripper_reading) {}

    auto get_gripper_channel() -> MockChannelType& override {
        return gripper_channel;
    }
    auto get_z_channel() -> MockChannelType& override { return z_channel; }
    auto get_a_channel() -> MockChannelType& override { return a_channel; }

  private:
    MockChannelType z_channel;
    MockChannelType a_channel;
    MockChannelType gripper_channel;
};
}  // namespace adc
