#pragma once

#include "sensors/core/sensor_hardware_interface.hpp"
namespace test_mocks {
class MockSensorHardware : public sensor_hardware::SensorHardwareBase {
  public:
    auto set_sync() -> void override { sync_state = true; }
    auto reset_sync() -> void override { sync_state = false; }

    auto get_sync_state_mock() const -> bool { return sync_state; }

  private:
    bool sync_state = false;
};
};  // namespace test_mocks
