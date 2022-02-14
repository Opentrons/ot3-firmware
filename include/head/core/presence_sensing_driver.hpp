#pragma once

#include <array>

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"
#include "head/core/attached_tools.hpp"

namespace presence_sensing_driver {
using namespace can_ids;

class PresenceSensingDriver {
  public:
    explicit PresenceSensingDriver(adc::BaseADC& adc)
        : PresenceSensingDriver(adc, attached_tools::AttachedTools{}) {}
    PresenceSensingDriver(adc::BaseADC& adc,
                          attached_tools::AttachedTools current_tools)
        : adc_comms(adc), current_tools(current_tools) {}
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
    auto get_current_tools() -> attached_tools::AttachedTools {
        return this->current_tools;
    }

    void set_current_tools(attached_tools::AttachedTools tools) {
        this->current_tools = tools;
    }

  private:
    adc::BaseADC& adc_comms;
    attached_tools::AttachedTools current_tools;
};

}  // namespace presence_sensing_driver
