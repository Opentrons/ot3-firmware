#pragma once

#include <array>
#include <utility>

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "head/core/adc.hpp"
#include "head/core/attached_tools.hpp"

namespace presence_sensing_driver {
using namespace can::ids;

class PresenceSensingDriver {
  public:
    explicit PresenceSensingDriver(adc::BaseADC& adc)
        : PresenceSensingDriver(adc, attached_tools::AttachedTools{}) {}
    PresenceSensingDriver(adc::BaseADC& adc,
                          attached_tools::AttachedTools current_tools)
        : adc_comms(adc), current_tools(current_tools) {}
    auto get_readings() -> adc::MillivoltsReadings {
        return adc_comms.get_voltages();
    }
    auto get_current_tools() -> attached_tools::AttachedTools {
        return this->current_tools;
    }

    void set_current_tools(attached_tools::AttachedTools tools) {
        this->current_tools = tools;
    }

    /**
     * Determine if two attached tool structs are different enough from each
     * other that upstream should be notified of a change.
     *
     * In some cases, hardware tolerances can be slightly incorrect, so readings
     * can bounce between some valid reading and one of the holes in the ranges.
     * This causes extreme canbus loading and bad behavior because upstream
     * keeps getting notified that things break.
     *
     * To prevent this, we can suppress notifications for when we see a change
     * from a valid setting to an unknown value.
     *
     * This function therefore returns a (bool, AttachedTools) tuple. The bool
     * is true if the new value is different from what was previously cached.
     *
     * After the call, the internal tool cache will be the same as what is
     * returned.
     */

    [[nodiscard]] auto update_tools()
        -> std::pair<bool, attached_tools::AttachedTools> {
        auto new_tools = attached_tools::AttachedTools(get_readings());
        bool notify = should_send_notification(current_tools, new_tools);
        current_tools = new_tools;
        return std::make_pair(notify, current_tools);
    }

  private:
    [[nodiscard]] constexpr static auto tool_attached_changed(
        can::ids::ToolType old_tool, can::ids::ToolType new_tool) -> bool {
        return old_tool != new_tool;
    }

    [[nodiscard]] constexpr static auto should_send_notification(
        const attached_tools::AttachedTools& old_tools,
        const attached_tools::AttachedTools& new_tools) -> bool {
        return tool_attached_changed(old_tools.z_motor, new_tools.z_motor) ||
               tool_attached_changed(old_tools.a_motor, new_tools.a_motor) ||
               tool_attached_changed(old_tools.gripper, new_tools.gripper);
    }

    adc::BaseADC& adc_comms;
    attached_tools::AttachedTools current_tools;
};

}  // namespace presence_sensing_driver
