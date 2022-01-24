#pragma once

#include <array>
#include <vector>

#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"

namespace presence_sensing_driver {
static std::vector<adc::Tool> OT3ToolList{
    {adc::ToolType::UNDEFINED, 0, 0},
    {adc::ToolType::PIPETTE96CHAN, PIPETTE_96_CHAN_DETECTION_UPPER_BOUND,
     PIPETTE_96_CHAN_DETECTION_LOWER_BOUND},
    {adc::ToolType::GRIPPER, GRIPPER_DETECTION_UPPER_BOUND,
     GRIPPER_DETECTION_LOWER_BOUND}};

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

    auto get_tool(uint16_t reading) -> adc::Tool {
        adc::Tool td = {adc::ToolType::UNDEFINED, 0, 0};
        for (size_t i = 0; i < OT3ToolList.size(); i++)
            if ((reading < OT3ToolList[i].detection_upper_bound) &&
                (reading >= OT3ToolList[i].detection_lower_bound))
                return OT3ToolList[i];
        return td;
    }

  private:
    adc::BaseADC& adc_comms;
};

}  // namespace presence_sensing_driver
