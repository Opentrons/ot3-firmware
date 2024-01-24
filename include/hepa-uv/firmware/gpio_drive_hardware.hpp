#pragma once

#include "common/firmware/gpio.hpp"

namespace gpio_drive_hardware {

struct GpioDrivePins {
    gpio::PinConfig door_open;
    gpio::PinConfig reed_switch;
    gpio::PinConfig hepa_push_button;
    gpio::PinConfig uv_push_button;
    gpio::PinConfig hepa_on_off;
    gpio::PinConfig uv_on_off;
};

}  // namespace gpio_drive_hardware