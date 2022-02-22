#pragma once

#include "common/simulation/adc_channel.hpp"
#include "head/core/adc.hpp"

namespace adc {
class SimADC : public BaseADC {
  public:
    using SimChannel = SimADCChannel<BaseADC::ADC_FULLSCALE_MV,
                                     BaseADC::ADC_FULLSCALE_READING>;

  protected:
    auto get_gripper_channel() -> SimChannel& override {
        return gripper_channel;
    }
    auto get_z_channel() -> SimChannel& override { return z_channel; }
    auto get_a_channel() -> SimChannel& override { return a_channel; }

  private:
    SimChannel z_channel;
    SimChannel a_channel;
    SimChannel gripper_channel;
};
};  // namespace adc
