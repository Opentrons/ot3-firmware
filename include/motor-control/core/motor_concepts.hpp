#pragma once

#include <concepts>

namespace motor_concepts {

template <typename GM>
concept GenericMotor = requires(GM gm) {
    gm.pending_move_queue;
    gm.driver;
    gm.motion_controller;
};

}  // namespace motor_concepts