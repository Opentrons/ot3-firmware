#pragma once

#include "common/simulation/state_manager.hpp"
#include "head/core/attached_tools.hpp"
#include "head/core/presence_sensing_driver.hpp"

namespace presence_sensing_driver {

using StateManager = state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>;
using StateManagerHandle = std::shared_ptr<StateManager>;

class PresenceSensingSimulator : public PresenceSensingDriver {
  public:
    PresenceSensingSimulator() {}
    auto get_readings() -> attached_tools::MountPinMeasurements override {
        if (_state_mgr) {
            auto values = _state_mgr->attached_instruments();
            return attached_tools::MountPinMeasurements{
                .left_present = values.pipette_left,
                .right_present = values.pipette_right,
                .gripper_present = values.gripper,
            };
        }
        return attached_tools::MountPinMeasurements{.left_present = false,
                                                    .right_present = false,
                                                    .gripper_present = false};
    }
    ~PresenceSensingSimulator() override = default;

    auto provide_state_manager(StateManagerHandle handle) -> void {
        _state_mgr = handle;
    }

  private:
    StateManagerHandle _state_mgr = nullptr;
};
}  // namespace presence_sensing_driver
