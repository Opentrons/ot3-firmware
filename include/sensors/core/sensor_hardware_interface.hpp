#pragma once

#include <functional>
#include <optional>

#include "can/core/ids.hpp"
#include "common/firmware/gpio.hpp"

namespace sensors {
namespace hardware {

struct SensorHardwareConfiguration {
    gpio::PinConfig sync_in{};
    gpio::PinConfig sync_out{};
    std::optional<gpio::PinConfig> data_ready = std::nullopt;
    std::optional<gpio::PinConfig> tip_sense = std::nullopt;
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
    virtual auto check_tip_presence() -> bool = 0;
    virtual auto set_sync(can::ids::SensorId sensor) -> void = 0;
    virtual auto reset_sync(can::ids::SensorId sensor) -> void = 0;
    virtual auto set_sync_required(can::ids::SensorId sensor, bool required) -> void = 0;
    virtual auto reset_sync_required(can::ids::SensorId sensor, bool required) -> void = 0;
};

struct SensorHardwareContainer {
    SensorHardwareBase& primary;
    SensorHardwareBase& secondary;
};

};  // namespace hardware
};  // namespace sensors
