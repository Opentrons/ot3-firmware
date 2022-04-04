#pragma once

namespace sensor_hardware {

/** abstract sensor hardware device for a sync line */
class SensorHardwareBase {
  public:
    SensorHardwareBase() = default;
    virtual ~SensorHardwareBase() = default;
    SensorHardwareBase(const SensorHardwareBase&) = default;
    auto operator=(const SensorHardwareBase&) -> SensorHardwareBase& = default;
    SensorHardwareBase(SensorHardwareBase&&) = default;
    auto operator=(SensorHardwareBase&&) -> SensorHardwareBase& = default;

    virtual auto set_sync() -> void = 0;
    virtual auto reset_sync() -> void = 0;
};
};  // namespace sensor_hardware
