#pragma once
#include <array>
#include <functional>

#include "sensors/core/sensor_hardware_interface.hpp"

namespace sensors {
namespace hardware {

enum class SensorIdBitMask : uint8_t {
    S0 = 0x01,
    S1 = 0x02,
    UNUSED = 0x00,
    BOTH = 0x03,
};

static auto get_mask_from_id(can::ids::SensorId sensor) -> SensorIdBitMask{
    switch(sensor) {
        case can::ids::SensorId::BOTH:
            return SensorIdBitMask::BOTH;
        case can::ids::SensorId::S0 :
            return SensorIdBitMask::S0;
        case can::ids::SensorId::S1 :
            return SensorIdBitMask::S1;
        case can::ids::SensorId::UNUSED :
        default :
            return SensorIdBitMask::UNUSED;
    }
}

class SensorHardware : public SensorHardwareBase {
  public:
    SensorHardware(sensors::hardware::SensorHardwareConfiguration hardware)
        : hardware(hardware) {}
    auto set_sync() -> void override { gpio::set(hardware.sync_out); }
    auto reset_sync() -> void override { gpio::reset(hardware.sync_out); }
    auto check_tip_presence() -> bool override {
        if (hardware.tip_sense.has_value()) {
            return gpio::is_set(hardware.tip_sense.value());
        }
        return false;
    }
    auto set_sync(can::ids::SensorId sensor) -> void {
        sync_state_mask |= get_mask_from_id(sensor);
        if (sync_state_mask & set_sync_required_mask == set_sync_required_mask) {
            set_sync();
        }
    }

    auto reset_sync(can::ids::SensorId sensor) -> void {
        sync_state_mask &= 0xFF ^ get_mask_from_id(sensor);
        if (sync_state_mask & set_sync_required_mask != set_sync_required_mask) {
            reset_sync();
        }
    }

    auto set_sync_required(can::ids::SensorId sensor, bool required) -> void {
        uint8_t applied_mask = get_mask_from_id(sensor);
        if (!required) {
            applied_mask ^= 0xFF;
        }
        set_sync_required_mask &= applied_mask;
    }

    uint8_t set_sync_required_mask = 0x00;
    uint8_t sync_state_mask = 0x00;

};

};  // namespace hardware
};  // namespace sensors
