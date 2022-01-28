#pragma once

#include <array>
#include <concepts>

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"

namespace presence_sensing_driver {

using namespace can_ids;

constexpr uint16_t PIPETTE_384_CHAN_Z_CARRIER_DETECTION_UPPER_BOUND = 1883;
constexpr uint16_t PIPETTE_384_CHAN_Z_CARRIER_DETECTION_LOWER_BOUND = 1851;

constexpr uint16_t PIPETTE_96_CHAN_Z_CARRIER_DETECTION_UPPER_BOUND = 1434;
constexpr uint16_t PIPETTE_96_CHAN_Z_CARRIER_DETECTION_LOWER_BOUND = 1400;

constexpr uint16_t PIPETTE_SINGLE_Z_CARRIER_DETECTION_UPPER_BOUND = 460;
constexpr uint16_t PIPETTE_SINGLE_Z_CARRIER_DETECTION_LOWER_BOUND = 444;

constexpr uint16_t PIPETTE_MULTI_Z_CARRIER_DETECTION_UPPER_BOUND = 945;
constexpr uint16_t PIPETTE_MULTI_Z_CARRIER_DETECTION_LOWER_BOUND = 918;

constexpr uint16_t PIPETTE_SINGLE_A_CARRIER_DETECTION_UPPER_BOUND = 2389;
constexpr uint16_t PIPETTE_SINGLE_A_CARRIER_DETECTION_LOWER_BOUND = 2362;

constexpr uint16_t PIPETTE_MULTI_A_CARRIER_DETECTION_UPPER_BOUND = 2860;
constexpr uint16_t PIPETTE_MULTI_A_CARRIER_DETECTION_LOWER_BOUND = 2844;

constexpr uint16_t GRIPPER_DETECTION_UPPER_BOUND = 999;
constexpr uint16_t GRIPPER_DETECTION_LOWER_BOUND = 536;

enum Carrier : uint8_t {
    UNDEFINED_CARRIER = 0x00,
    Z_CARRIER = 0x01,
    A_CARRIER = 0x02,
};

struct Tool {
    ToolType tool_type;
    Carrier tool_carrier;
    uint16_t detection_upper_bound;
    uint16_t detection_lower_bound;
};

static std::array<Tool, 8> OT3ToolList{
    {{can_ids::ToolType::UNDEFINED_TOOL, UNDEFINED_CARRIER, 0, 0},
     {can_ids::ToolType::PIPETTE96CHAN, Z_CARRIER,
      PIPETTE_384_CHAN_Z_CARRIER_DETECTION_UPPER_BOUND,
      PIPETTE_384_CHAN_Z_CARRIER_DETECTION_LOWER_BOUND},
     {can_ids::ToolType::PIPETTE96CHAN, Z_CARRIER,
      PIPETTE_96_CHAN_Z_CARRIER_DETECTION_UPPER_BOUND,
      PIPETTE_96_CHAN_Z_CARRIER_DETECTION_LOWER_BOUND},
     {can_ids::ToolType::PIPETTESINGLE, Z_CARRIER,
      PIPETTE_SINGLE_Z_CARRIER_DETECTION_UPPER_BOUND,
      PIPETTE_SINGLE_Z_CARRIER_DETECTION_LOWER_BOUND},
     {can_ids::ToolType::PIPETTEMULTI, Z_CARRIER,
      PIPETTE_MULTI_Z_CARRIER_DETECTION_UPPER_BOUND,
      PIPETTE_MULTI_Z_CARRIER_DETECTION_LOWER_BOUND},
     {can_ids::ToolType::PIPETTESINGLE, A_CARRIER,
      PIPETTE_SINGLE_A_CARRIER_DETECTION_UPPER_BOUND,
      PIPETTE_SINGLE_A_CARRIER_DETECTION_LOWER_BOUND},
     {can_ids::ToolType::PIPETTEMULTI, A_CARRIER,
      PIPETTE_MULTI_A_CARRIER_DETECTION_UPPER_BOUND,
      PIPETTE_MULTI_A_CARRIER_DETECTION_LOWER_BOUND},
     {can_ids::ToolType::GRIPPER, UNDEFINED_CARRIER,
      GRIPPER_DETECTION_UPPER_BOUND, GRIPPER_DETECTION_LOWER_BOUND}}};

struct AttachedTool {
    ToolType z_motor;
    ToolType a_motor;
    ToolType gripper;
};

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

    auto static get_tool(adc::MillivoltsReadings reading) -> AttachedTool {
        auto at = AttachedTool{.z_motor = can_ids::ToolType::UNDEFINED_TOOL,
                               .a_motor = can_ids::ToolType::UNDEFINED_TOOL,
                               .gripper = can_ids::ToolType::UNDEFINED_TOOL};
        for (auto& element : OT3ToolList) {
            if ((reading.z_motor < element.detection_upper_bound) &&
                (reading.z_motor >= element.detection_lower_bound) &&
                (element.tool_carrier == Z_CARRIER)) {
                at.z_motor = element.tool_type;
            }
            if ((reading.a_motor < element.detection_upper_bound) &&
                (reading.a_motor >= element.detection_lower_bound) &&
                (element.tool_carrier == A_CARRIER)) {
                at.a_motor = element.tool_type;
            }
            if ((reading.gripper < element.detection_upper_bound) &&
                (reading.gripper >= element.detection_lower_bound) &&
                (element.tool_carrier == UNDEFINED_CARRIER)) {
                at.gripper = element.tool_type;
            }
        }

        return at;
    }

  private:
    adc::BaseADC& adc_comms;
};

}  // namespace presence_sensing_driver
