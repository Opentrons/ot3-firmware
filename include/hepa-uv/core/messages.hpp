#pragma once

#include "can/core/messages.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"

namespace interrupt_task_messages {

/**
 * A message sent when external interurpts are triguered
 */
struct GPIOInterruptChanged {
    uint16_t pin;
    uint8_t state;
};

using TaskMessage = std::variant<std::monostate, GPIOInterruptChanged>;

}  // namespace interrupt_task_messages

namespace led_control_task_messages {

/**
 * A message to change the leds on the push buttons
 */
struct PushButtonLED {
	PUSH_BUTTON_TYPE button;
    double r = 0, g = 0, b = 0, w = 0;
};

using TaskMessage = std::variant<std::monostate, PushButtonLED>;

}  // namespace led_control_task_messages
