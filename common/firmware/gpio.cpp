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

auto gpio::debouce_update(bool new_state, std::atomic_bool& value,
                          std::atomic_bool& value_bounce) -> void {
    // only set the state if the bounce matches the current gpio_is_set
    // on the first state change it won't match but on the second tick it will
    // and we can set it to the new state.
    if (new_state == value_bounce) {
        value = new_state;
    }
    value_bounce = new_state;
}
