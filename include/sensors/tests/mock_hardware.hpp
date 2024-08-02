#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"
namespace test_mocks {
class MockSensorHardware : public sensors::hardware::SensorHardwareBase {
  public:
    MockSensorHardware(
                   sensors::hardware::SensorHardwareVersionSingleton& version_wrapper, sensors::hardware::SensorHardwareSyncControlSingleton& sync_control)
        : sensors::hardware::SensorHardwareBase{version_wrapper, sync_control} {}
    auto set_sync() -> void override {
        sync_state = true;
        sync_set_calls++;
    }
    auto reset_sync() -> void override {
        sync_state = false;
        sync_reset_calls++;
    }
    auto check_tip_presence() -> bool override { return false; }

    auto get_sync_state_mock() const -> bool { return sensors::hardware::SensorHardwareBase::mask_satisfied(); }
    auto get_sync_set_calls() const -> uint32_t { return sync_set_calls; }
    auto get_sync_reset_calls() const -> uint32_t { return sync_reset_calls; }

    auto set_sync(can::ids::SensorId sensor) -> void {
        sensors::hardware::SensorHardwareBase::set_sync(sensor);
    }
    auto reset_sync(can::ids::SensorId sensor) -> void {
        sensors::hardware::SensorHardwareBase::reset_sync(sensor);
    }
    auto set_sync_enabled(can::ids::SensorId sensor, bool enabled) -> void {
        sensors::hardware::SensorHardwareBase::set_sync_enabled(sensor,
                                                                enabled);
    }
    auto set_sync_required(can::ids::SensorId sensor, bool required) -> void {
        sensors::hardware::SensorHardwareBase::set_sync_required(sensor,
                                                                 required);
    }

  private:
    bool sync_state = false;
    uint32_t sync_set_calls = 0;
    uint32_t sync_reset_calls = 0;
};
};  // namespace test_mocks
