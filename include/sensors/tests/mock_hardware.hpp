#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"
namespace test_mocks {
class MockSensorHardware : public sensors::hardware::SensorHardwareBase {
  public:
    auto set_sync_out() -> void override {
        sync_state = true;
        sync_set_calls++;
    }
    auto reset_sync_out() -> void override {
        sync_state = false;
        sync_reset_calls++;
    }
    auto check_data_ready() -> bool override { return data_ready; }
    void change_data_ready_value(bool value) -> void { data_ready = value; }
    auto get_sync_state_mock() const -> bool { return sync_state; }
    auto get_sync_set_calls() const -> uint32_t { return sync_set_calls; }
    auto get_sync_reset_calls() const -> uint32_t { return sync_reset_calls; }

  private:
    bool sync_state = false;
    bool data_ready = false;
    uint32_t sync_set_calls = 0;
    uint32_t sync_reset_calls = 0;
};
};  // namespace test_mocks
