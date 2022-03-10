#pragma once

#include <variant>

#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace brushed_motor_hardware {
class MotorHardwareIface;
};

namespace brushed_motor_class {

using namespace motor_messages;
using namespace freertos_message_queue;

template <lms::MotorMechanicalConfig MEConfig>
struct BrushedMotor {
    using GenericQueue = FreeRTOSMessageQueue<Move>;

    /**
     * Construct a brushed motor
     *
     * @param lms_config Linear motion system configuration
     * @param hardware_iface Hardware interface
     * @param constraints Motion constraints
     * @param queue Input message queue containing motion commands.
     * commands.
     */
    Motor(lms::LinearMotionSystemConfig<MEConfig> lms_config,
          motor_hardware::MotorHardwareIface& hardware_iface,
          MotionConstraints constraints, GenericQueue& queue)

        : pending_move_queue(queue),
          motor_driver(hardware_iface),
          motion_controller{lms_config, hardware_iface, constraints,
                            pending_move_queue} {}
    GenericQueue& pending_move_queue;
    motion_controller::MotionController<MEConfig> motion_controller;

    Motor(const Motor&) = delete;
    auto operator=(const Motor&) -> Motor& = delete;
    Motor(Motor&&) = delete;
    auto operator=(Motor&&) -> Motor&& = delete;
    ~Motor() = default;
};

}  // namespace brushed_motor_class
