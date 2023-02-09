#include "common/firmware/gpio.hpp"

#include "gpio.h"

auto gpio::set(const gpio::PinConfig& pc) -> void {
    return gpio_set(pc.port, pc.pin, pc.active_setting);
}

auto gpio::reset(const gpio::PinConfig& pc) -> void {
    return gpio_reset(pc.port, pc.pin, pc.active_setting);
}

auto gpio::is_set(const gpio::PinConfig& pc) -> bool {
    return gpio_is_set(pc.port, pc.pin, pc.active_setting);
}
