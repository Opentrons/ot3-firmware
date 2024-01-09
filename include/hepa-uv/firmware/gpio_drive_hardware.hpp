#pragma once

#include "common/firmware/gpio.hpp"

namespace gpio_drive_hardware {

struct GpioDrivePins {
    gpio::PinConfig push_button_led;
    gpio::PinConfig push_button;
};

}  // namespace gpio_drive_hardware