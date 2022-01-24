#pragma once

#include <array>
#include <vector>

#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"

namespace presence_sensing_driver {

constexpr uint16_t PIPETTE_96_CHAN_DETECTION_UPPER_BOUND = 999;
constexpr uint16_t PIPETTE_96_CHAN_DETECTION_LOWER_BOUND = 666;

constexpr uint16_t GRIPPER_DETECTION_UPPER_BOUND = 999;
constexpr uint16_t GRIPPER_DETECTION_LOWER_BOUND = 666;

enum ToolType : uint8_t {
    UNDEFINED = 0x00,
    PIPETTE96CHAN = 0x00,
    GRIPPER = 0x01,
};

struct Tool {
    ToolType tool_type;
    uint16_t detection_upper_bound;
    uint16_t detection_lower_bound;
};

static std::vector<Tool> OT3ToolList{
    {ToolType::UNDEFINED, 0, 0},
    {ToolType::PIPETTE96CHAN, PIPETTE_96_CHAN_DETECTION_UPPER_BOUND,
     PIPETTE_96_CHAN_DETECTION_LOWER_BOUND},
    {ToolType::GRIPPER, GRIPPER_DETECTION_UPPER_BOUND,
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

    auto static get_tool(uint16_t reading) -> Tool {
        Tool td = {ToolType::UNDEFINED, 0, 0};
        for (auto& element : OT3ToolList) {
            if ((reading < element.detection_upper_bound) &&
                (reading >= element.detection_lower_bound)) {
                return element;
            }
        }
        return td;
    }

  private:
    adc::BaseADC& adc_comms;
};

}  // namespace presence_sensing_driver
