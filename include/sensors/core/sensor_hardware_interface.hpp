#pragma once

#include <functional>
#include <optional>

#include "can/core/ids.hpp"
#include "common/firmware/gpio.hpp"
#include "sensors/core/utils.hpp"

namespace sensors {
namespace hardware {

struct SensorHardwareConfiguration {
    gpio::PinConfig sync_in{};
    gpio::PinConfig sync_out{};
    std::optional<gpio::PinConfig> data_ready = std::nullopt;
    std::optional<gpio::PinConfig> tip_sense = std::nullopt;
};

enum class SensorIdBitMask : uint8_t {
    S0 = 0x01,
    S1 = 0x02,
    UNUSED = 0x00,
    BOTH = 0x03,
};

static auto get_mask_from_id(can::ids::SensorId sensor) -> uint8_t {
    auto mask_enum = SensorIdBitMask::UNUSED;
    switch (sensor) {
        case can::ids::SensorId::BOTH:
            mask_enum = SensorIdBitMask::BOTH;
            break;
        case can::ids::SensorId::S0:
            mask_enum = SensorIdBitMask::S0;
            break;
        case can::ids::SensorId::S1:
            mask_enum = SensorIdBitMask::S1;
            break;
        case can::ids::SensorId::UNUSED:
        default:
            mask_enum = SensorIdBitMask::UNUSED;
    }
    return static_cast<uint8_t>(mask_enum);
}

class SensorHardwareVersionSingleton {
  public:
    SensorHardwareVersionSingleton() = default;
    virtual ~SensorHardwareVersionSingleton() = default;
    SensorHardwareVersionSingleton(const SensorHardwareVersionSingleton&) =
        default;
    auto operator=(const SensorHardwareVersionSingleton&)
        -> SensorHardwareVersionSingleton& = default;
    SensorHardwareVersionSingleton(SensorHardwareVersionSingleton&&) = default;
    auto operator=(SensorHardwareVersionSingleton&&)
        -> SensorHardwareVersionSingleton& = default;

    void set_board_rev(utils::SensorBoardRev rev) { b_revision = rev; }

    auto get_board_rev() -> utils::SensorBoardRev { return b_revision; }

  private:
    utils::SensorBoardRev b_revision = utils::SensorBoardRev::VERSION_0;
};

/** abstract sensor hardware device for a sync line */
class SensorHardwareBase {
  public:
    SensorHardwareBase(SensorHardwareVersionSingleton& version_wrapper)
        : version_wrapper{version_wrapper} {}
    virtual ~SensorHardwareBase() = default;
    SensorHardwareBase(const SensorHardwareBase&) = default;
    auto operator=(const SensorHardwareBase&) -> SensorHardwareBase& = delete;
    SensorHardwareBase(SensorHardwareBase&&) = default;
    auto operator=(SensorHardwareBase&&) -> SensorHardwareBase& = delete;

    virtual auto set_sync() -> void = 0;
    virtual auto reset_sync() -> void = 0;
    virtual auto check_tip_presence() -> bool = 0;

    [[nodiscard]] auto mask_satisfied() const -> bool {
        if (set_sync_required_mask !=
            static_cast<uint8_t>(SensorIdBitMask::UNUSED)) {
            // if anything is "required" only sync when they are all triggered
            return (sync_state_mask & set_sync_required_mask) ==
                   set_sync_required_mask;
        }
        return (sync_state_mask & set_sync_enabled_mask) != 0;
    }

    auto set_sync(can::ids::SensorId sensor) -> void {
        // force the bit for this sensor to 1
        sync_state_mask |= get_mask_from_id(sensor);
        if (mask_satisfied()) {
            set_sync();
        }
    }

    auto reset_sync(can::ids::SensorId sensor) -> void {
        // force the bit for this sensor to 0
        sync_state_mask &= 0xFF ^ get_mask_from_id(sensor);
        if (!mask_satisfied()) {
            reset_sync();
        }
    }

    auto set_sync_enabled(can::ids::SensorId sensor, bool enabled) -> void {
        uint8_t applied_mask = get_mask_from_id(sensor);
        if (!enabled) {
            // force enabled bit to 0
            set_sync_enabled_mask &= 0xFF ^ applied_mask;
        } else {
            // force enabled bit to 1
            set_sync_enabled_mask |= applied_mask;
        }
        // update sync state now that requirements are different
        if (mask_satisfied()) {
            set_sync();
        } else {
            reset_sync();
        }
    }

    auto set_sync_required(can::ids::SensorId sensor, bool required) -> void {
        uint8_t applied_mask = get_mask_from_id(sensor);
        if (!required) {
            // force required bit to 0
            set_sync_required_mask &= 0xFF ^ applied_mask;
        } else {
            // force required bit to 1
            set_sync_required_mask |= applied_mask;
        }
        // update sync state now that requirements are different
        if (mask_satisfied()) {
            set_sync();
        } else {
            reset_sync();
        }
    }
    auto get_board_rev() -> utils::SensorBoardRev {
        return version_wrapper.get_board_rev();
    }

  private:
    uint8_t set_sync_required_mask = 0x00;
    uint8_t set_sync_enabled_mask = 0x00;
    uint8_t sync_state_mask = 0x00;
    SensorHardwareVersionSingleton& version_wrapper;
};

struct SensorHardwareContainer {
    SensorHardwareBase& primary;
    SensorHardwareBase& secondary;
};

};  // namespace hardware
};  // namespace sensors
