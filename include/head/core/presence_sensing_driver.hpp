#pragma once

#include <array>
#include <utility>

#include "can/core/ids.hpp"
#include "common/core/bit_utils.hpp"
#include "head/core/attached_tools.hpp"

namespace presence_sensing_driver {
using namespace can::ids;

class PresenceSensingDriver {
  public:
    PresenceSensingDriver()
        : PresenceSensingDriver(attached_tools::AttachedTools{}) {}
    explicit PresenceSensingDriver(const PresenceSensingDriver&) = default;
    auto operator=(const PresenceSensingDriver&)
        -> PresenceSensingDriver& = default;
    explicit PresenceSensingDriver(PresenceSensingDriver&&) = default;
    auto operator=(PresenceSensingDriver&&) -> PresenceSensingDriver& = default;
    explicit PresenceSensingDriver(attached_tools::AttachedTools current_tools)
        : current_tools(current_tools) {}
    virtual auto get_readings() -> attached_tools::MountPinMeasurements = 0;
    virtual ~PresenceSensingDriver() = default;

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
     * This function returns a (bool, AttachedTools) tuple. The bool
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

    attached_tools::AttachedTools current_tools;
};

}  // namespace presence_sensing_driver
