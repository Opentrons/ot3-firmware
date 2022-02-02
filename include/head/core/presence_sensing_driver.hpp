#pragma once

#include <array>

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"
#include "head/core/tool_list.hpp"

using namespace ot3_tool_list;
namespace presence_sensing_driver {
using namespace can_ids;

class PresenceSensingDriver {
  public:
    PresenceSensingDriver(adc::BaseADC& adc,
                          ot3_tool_list::AttachedTool current_tools)
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
    auto getCurrentTools() -> ot3_tool_list::AttachedTool {
        return this->current_tools;
    }

    void setCurrentTools(ot3_tool_list::AttachedTool tmp) {
        this->current_tools.z_motor = tmp.z_motor;
        this->current_tools.a_motor = tmp.a_motor;
        this->current_tools.gripper = tmp.gripper;
    }

  private:
    adc::BaseADC& adc_comms;
    ot3_tool_list::AttachedTool current_tools;
};

}  // namespace presence_sensing_driver
