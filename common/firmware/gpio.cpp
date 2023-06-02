#include "common/firmware/gpio.hpp"

#include "gpio.h"

__attribute__((section( ".ccmram" )))
auto gpio::set(const gpio::PinConfig& pc) -> void {
    return gpio_set(pc.port, pc.pin, pc.active_setting);
}

__attribute__((section( ".ccmram" )))
auto gpio::reset(const gpio::PinConfig& pc) -> void {
    return gpio_reset(pc.port, pc.pin, pc.active_setting);
}

__attribute__((section( ".ccmram" )))
auto gpio::is_set(const gpio::PinConfig& pc) -> bool {
    return gpio_is_set(pc.port, pc.pin, pc.active_setting);
}
