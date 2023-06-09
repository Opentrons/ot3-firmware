#pragma once

#include "common/firmware/gpio.hpp"

namespace gpio_drive_hardware {

struct GpioDrivePins {
    gpio::PinConfig estop_out;
    gpio::PinConfig sync_out;
    gpio::PinConfig estop_in;
    gpio::PinConfig estop_aux1_det;
    gpio::PinConfig estop_aux2_det;
    gpio::PinConfig door_open;
    gpio::PinConfig aux1_present;
    gpio::PinConfig aux2_present;
    gpio::PinConfig aux1_id;
    gpio::PinConfig aux2_id;
    gpio::PinConfig heartbeat_led;
};

}  // namespace gpio_drive_hardware