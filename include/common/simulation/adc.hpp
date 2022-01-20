#pragma once

#include "head/core/adc.hpp"

namespace adc {

class SimADC : public adc::BaseADC {
    auto get_readings() -> adc::RawADCReadings { return adc::RawADCReadings{}; }
};

}  // namespace adc