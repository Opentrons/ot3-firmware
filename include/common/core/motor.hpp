#pragma once

#include <concepts>
#include <cstdint>

namespace motor_protocol {

template <class Motor>
concept MotorProtocol = requires(Motor m, uint32_t speed) {
    {m.motion_controller.set_speed(speed)};
};

}  // namespace motor_protocol