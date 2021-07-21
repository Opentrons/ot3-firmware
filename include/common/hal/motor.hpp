#pragma once

#include <concepts>

namespace motor_protocol {

template <class Motor>
concept MotorProtocol = requires(Motor m, uint32_t speed) {
    {m.set_speed(speed)};
    { m.get_speed() }
    ->std::same_as<uint32_t>;
};

}  // namespace motor_protocol