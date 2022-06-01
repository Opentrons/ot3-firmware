#pragma once

#include "common/firmware/gpio.hpp"

namespace sensors {
namespace hardware {

struct SensorHardwareConfiguration {
    gpio::PinConfig sync_in;
    gpio::PinConfig sync_out;
    gpio::PinConfig data_ready;
};

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
    virtual auto check_data_ready() -> bool = 0;
};
};  // namespace hardware
};  // namespace sensors
