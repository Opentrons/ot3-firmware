#pragma once

#include "common/core/adc.hpp"

auto voltage_read = adc::VoltageRead{
        .z_motor = 666,
        .a_motor = 666,
        .gripper = 666};

namespace adc {
    class MockADC : public adc::BaseADC {
    public:
        auto get_readings() -> adc::VoltageRead {return voltage_read;}
    };
}