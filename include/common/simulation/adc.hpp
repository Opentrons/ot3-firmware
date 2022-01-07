#pragma once

#include "common/core/adc.hpp"

namespace adc {

class SimADC : public adc::BaseADC {
    auto get_readings() -> adc::VoltageRead { return adc::VoltageRead{}; }
};

}  // namespace adc