#include "hepa-uv/firmware/led_control_hardware.hpp"

#include "hepa-uv/core/led_control_task.hpp"
#include "hepa-uv/firmware/led_hardware.h"

using namespace led_control_hardware;

void LEDControlHardware::set_button_led_power(uint8_t button, uint32_t r,
                                              uint32_t g, uint32_t b,
                                              uint32_t w) {
    set_button_led_pwm(static_cast<PUSH_BUTTON_TYPE>(button), r, g, b, w);
}