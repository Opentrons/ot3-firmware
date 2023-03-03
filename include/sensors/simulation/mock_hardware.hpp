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
    std::array<std::function<void()>, 5> data_ready_callbacks = {};
    auto add_data_ready_callback(std::function<void()> callback)
        -> bool override {
        for (auto &callback_function : data_ready_callbacks) {
            if (callback_function) {
                continue;
            }
            callback_function = callback;
            return true;
        }
        return false;
    }

    auto data_ready() -> void {
        for (auto &callback_function : data_ready_callbacks) {
            if (callback_function) {
                callback_function();
            }
        }
    }

    void provide_state_manager(StateManagerHandle handle) {
        _state_manager = handle;
    }

  private:
    StateManagerHandle _state_manager = nullptr;
};
};  // namespace sim_mocks
