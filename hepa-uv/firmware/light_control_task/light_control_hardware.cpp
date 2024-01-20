#include "hepa-uv/firmware/light_control_hardware.hpp"

#include "hepa-uv/core/light_control_task.hpp"
#include "hepa-uv/firmware/led_hardware.h"

using namespace light_control_hardware;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto LightControlHardware::initialize() -> void { button_led_hw_initialize_leds(); }

void LightControlHardware::set_led_power(uint8_t button, uint8_t led, uint32_t duty_cycle) {
    button_led_hw_update_pwm(duty_cycle, static_cast<LED_TYPE>(led), static_cast<PUSH_BUTTON_TYPE>(button));
}