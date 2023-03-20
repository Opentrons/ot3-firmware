#include "rear-panel/firmware/light_control_hardware.hpp"

#include "rear-panel/core/tasks/light_control_task.hpp"
#include "rear-panel/firmware/led_hardware.h"

using namespace light_control_hardware;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
auto LightControlHardware::initialize() -> void { led_hw_initialize_leds(); }

void LightControlHardware::set_led_power(uint8_t id, uint32_t duty_cycle) {
    led_hw_update_pwm(duty_cycle, static_cast<LED_DEVICE>(id));
}