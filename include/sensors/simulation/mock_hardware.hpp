#pragma once

#include "common/core/freertos_synchronization.hpp"
#include "common/simulation/state_manager.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"

using StateManager = state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>;
using StateManagerHandle = std::shared_ptr<StateManager>;

namespace sim_mocks {
class MockSensorHardware : public sensors::hardware::SensorHardwareBase {
  public:
    MockSensorHardware(
        sensors::hardware::SensorHardwareVersionSingleton& version_wrapper)
        : sensors::hardware::SensorHardwareBase{version_wrapper} {}
    auto set_sync() -> void override {
        if (_state_manager) {
            _state_manager->send_sync_msg(SyncPinState::HIGH);
        }
    }
    auto reset_sync() -> void override {
        if (_state_manager) {
            _state_manager->send_sync_msg(SyncPinState::LOW);
        }
    }
    auto check_tip_presence() -> bool override { return false; }

    void provide_state_manager(StateManagerHandle handle) {
        _state_manager = handle;
    }

  private:
    StateManagerHandle _state_manager = nullptr;
};
};  // namespace sim_mocks
