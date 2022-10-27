#pragma once

#include "common/core/freertos_synchronization.hpp"
#include "common/simulation/state_manager.hpp"
#include "sensors/core/sensor_hardware_interface.hpp"

using StateManager = state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>;
using StateManagerHandle = std::shared_ptr<StateManager>;

namespace test_mocks {
class MockSensorHardware : public sensors::hardware::SensorHardwareBase {
  public:
    auto set_sync() -> void override {
        sync_state = true;
        sync_set_calls++;
        if (_state_manager) {
            _state_manager->send_sync_msg(SyncPinState::HIGH);
        }
    }
    auto reset_sync() -> void override {
        sync_state = false;
        sync_reset_calls++;
        if (_state_manager) {
            _state_manager->send_sync_msg(SyncPinState::LOW);
        }
    }
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

    auto get_sync_state_mock() const -> bool { return sync_state; }
    auto get_sync_set_calls() const -> uint32_t { return sync_set_calls; }
    auto get_sync_reset_calls() const -> uint32_t { return sync_reset_calls; }

  private:
    bool sync_state = false;
    uint32_t sync_set_calls = 0;
    uint32_t sync_reset_calls = 0;
    StateManagerHandle _state_manager = nullptr;
};
};  // namespace test_mocks
