#pragma once

#include "head/core/adc.hpp"

namespace adc {
class MockADC : public adc::BaseADC {
  public:
    explicit MockADC(adc::RawADCReadings& raw_readings)
        : raw_readings(raw_readings) {}
    auto get_readings() -> adc::RawADCReadings { return raw_readings; }

  private:
    adc::RawADCReadings& raw_readings;
};
}  // namespace adc