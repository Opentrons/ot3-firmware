#pragma once

#include "can/core/messages.hpp"
#include "hepa-uv/core/constants.h"
#include "hepa-uv/firmware/gpio_drive_hardware.hpp"

/**
 * A message sent when external interurpts are triguered
 */
struct GPIOInterruptChanged {
    uint16_t pin;
    uint8_t state;
};

/*
Messages for the Hepa Task
*/
namespace hepa_task_messages {

using TaskMessage = std::variant<std::monostate, GPIOInterruptChanged,
                                 can::messages::GetHepaFanStateRequest,
                                 can::messages::SetHepaFanStateRequest>;

}  // namespace hepa_task_messages

/*
Messages for the UV Task
*/
namespace uv_task_messages {

struct SetUVLightState {
    uint8_t state;
    uint32_t timeout;
};

using TaskMessage =
    std::variant<std::monostate, GPIOInterruptChanged, SetUVLightState>;

};  // namespace uv_task_messages

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
