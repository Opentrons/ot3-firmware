#pragma once

#include "head/core/adc.hpp"

auto voltage_read =
    adc::MillivoltsReadings{.z_motor = 666, .a_motor = 666, .gripper = 666};

namespace adc {
class MockADC : public adc::BaseADC {
  public:
    auto get_readings() -> adc::MillivoltsReadings { return voltage_read; }
};
}  // namespace adc