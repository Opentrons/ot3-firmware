#pragma once

#include "common/core/bit_utils.hpp"
#include "common/core/adc.hpp"
#include "presence_sensing_driver_config.hpp"

namespace presence_sensing_driver {

using namespace presence_sensing_driver_config;

template <adc::has_get_reading ADCDriver>
class PresenceSensingDriver {
  public:
    explicit PresenceSensingDriver(ADCDriver& adc) : ADC_comms(adc) {}
    struct voltage_read get_readings() {
        return ADC_comms.get_readings();
    }

  private:
    ADCDriver ADC_comms;
};

}  // namespace motor_driver
