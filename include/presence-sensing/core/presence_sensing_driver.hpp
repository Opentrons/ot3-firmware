#pragma once

#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"

namespace presence_sensing_driver {

class PresenceSensingDriver {
  public:
    explicit PresenceSensingDriver(adc::BaseADC& adc) : adc_comms(adc) {}
    auto get_readings() -> adc::MillivoltsReadings {
        return adc_comms.get_readings();
    }

  private:
    adc::BaseADC& adc_comms;
};

}  // namespace presence_sensing_driver
