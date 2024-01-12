#include "common/firmware/gpio.hpp"

#include "gpio.h"

#ifdef ENABLE_CCMRAM
__attribute__((section( ".ccmram" )))
#endif
auto gpio::set(const gpio::PinConfig& pc)
    -> void {
    return gpio_set(pc.port, pc.pin, pc.active_setting);
}

#ifdef ENABLE_CCMRAM
__attribute__((section( ".ccmram" )))
#endif
auto gpio::reset(const gpio::PinConfig& pc)
    -> void {
    return gpio_reset(pc.port, pc.pin, pc.active_setting);
}

#ifdef ENABLE_CCMRAM
__attribute__((section( ".ccmram" )))
#endif
auto gpio::is_set(const gpio::PinConfig& pc)
    -> bool {
    return gpio_is_set(pc.port, pc.pin, pc.active_setting);
}