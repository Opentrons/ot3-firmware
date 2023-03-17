#include "rear-panel/firmware/light_control_hardware.hpp"

#include "rear-panel/core/tasks/light_control_task.hpp"
#include "rear-panel/firmware/led_hardware.h"

using namespace light_control_hardware;

auto LightControlHardware::initialize() -> void { led_hw_initialize_leds(); }

auto LightControlHardware::set_led_power(uint8_t id, uint32_t duty_cycle)
    -> void {
    led_hw_update_pwm(duty_cycle, static_cast<LED_DEVICE>(id));
}