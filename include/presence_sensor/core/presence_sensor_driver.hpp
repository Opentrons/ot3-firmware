#pragma once

#include "adc.hpp"
#include "common/core/bit_utils.hpp"

namespace presence_sensor_driver {

enum class Mode : uint8_t { WRITE = 0x80, READ = 0x0 };

template <adc::has_get_reading ADCDriver>
class PresenceSensorDriver {
  public:
    explicit PresenceSensorDriver(ADCDriver& adc) : ADC_comms(adc) {}
    struct voltage_read get_readings() {
        return ADC_comms.get_readings();
    }

  private:
    ADCDriver ADC_comms;
};

}  // namespace presence_sensor_driver
