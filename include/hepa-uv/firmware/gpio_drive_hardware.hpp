#pragma once

#include <optional>

#include "common/firmware/gpio.hpp"

namespace gpio_drive_hardware {

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct GpioDrivePins {
    gpio::PinConfig door_open;
    gpio::PinConfig reed_switch;
    gpio::PinConfig hepa_push_button;
    gpio::PinConfig uv_push_button;
    gpio::PinConfig hepa_on_off;
    gpio::PinConfig uv_on_off;
    std::optional<gpio::PinConfig> safety_relay_active = std::nullopt;
};

}  // namespace gpio_drive_hardware