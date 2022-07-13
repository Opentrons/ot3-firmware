#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"
namespace test_mocks {
class MockSensorHardware : public sensors::hardware::SensorHardwareBase {
  public:
    auto set_sync() -> void override {
        sync_state = true;
        sync_set_calls++;
    }
    auto reset_sync() -> void override {
        sync_state = false;
        sync_reset_calls++;
    }
    std::array<std::function<void()>, 5> data_ready_callbacks = {};
    auto add_data_ready_callback(std::function<void()> callback)
        -> bool override {
        for (auto &callback_function : data_ready_callbacks) {
            if (callback_function) {
                continue;
            }
            callback_function = callback;
            LOG("callback added");
            return true;
        }
        return false;
    }

    // may need to copy the callback function into here
    // gotta also call add_data_ready_callback in the constructor

    auto data_ready() -> void {
        for (auto &callback_function : data_ready_callbacks) {
            LOG("inside for loop");
            if (callback_function) {
                LOG("callback function exists");
                callback_function();
            }
        }
    }
    auto get_sync_state_mock() const -> bool { return sync_state; }
    auto get_sync_set_calls() const -> uint32_t { return sync_set_calls; }
    auto get_sync_reset_calls() const -> uint32_t { return sync_reset_calls; }

  private:
    bool sync_state = false;
    uint32_t sync_set_calls = 0;
    uint32_t sync_reset_calls = 0;
};
};  // namespace test_mocks
