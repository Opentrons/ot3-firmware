#pragma once

#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"

namespace presence_sensing_driver {

class PresenceSensingDriver {
  public:
    explicit PresenceSensingDriver(adc::BaseADC& adc) : adc_comms(adc) {}
    auto get_readings() -> adc::MillivoltsReadings {
        auto RawReadings = adc_comms.get_readings();
        auto voltage_read = adc::MillivoltsReadings{
            .z_motor =
                static_cast<uint16_t>((static_cast<float>(RawReadings.z_motor) *
                                       adc::FULLSCALE_VOLTAGE) /
                                      adc::ADC_FULLSCALE_OUTPUT),
            .a_motor =
                static_cast<uint16_t>((static_cast<float>(RawReadings.a_motor) *
                                       adc::FULLSCALE_VOLTAGE) /
                                      adc::ADC_FULLSCALE_OUTPUT),
            .gripper =
                static_cast<uint16_t>((static_cast<float>(RawReadings.gripper) *
                                       adc::FULLSCALE_VOLTAGE) /
                                      adc::ADC_FULLSCALE_OUTPUT)};
        return voltage_read;
    }

  private:
    adc::BaseADC& adc_comms;
};

}  // namespace presence_sensing_driver
