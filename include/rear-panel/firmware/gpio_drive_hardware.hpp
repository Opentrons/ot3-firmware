#pragma once

#include "common/firmware/gpio.hpp"

namespace gpio_drive_hardware {

struct GpioDrivePins {
    gpio::PinConfig estop_out;
    gpio::PinConfig sync_out;
};

}  // namespace gpio_drive_hardware