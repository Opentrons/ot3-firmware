#pragma once

#include "can/core/messages.hpp"
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
