#pragma once

#include "common/firmware/gpio.hpp"

namespace gpio_drive_hardware {

struct GpioDrivePins {
    gpio::PinConfig push_button_led;
    gpio::PinConfig push_button;
    gpio::PinConfig reed_switch;
    gpio::PinConfig door_open;
};

}  // namespace gpio_drive_hardware