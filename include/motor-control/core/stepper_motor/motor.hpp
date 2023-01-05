#pragma once

#include <variant>

#include "common/core/freertos_message_queue.hpp"
#include "motion_controller.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_hardware {
class MotorHardwareIface;
};

namespace motor_class {

using namespace motor_messages;
using namespace freertos_message_queue;

template <lms::MotorMechanicalConfig MEConfig>
struct Motor {
    using GenericQueue = FreeRTOSMessageQueue<Move>;
    using UpdatePositionQueue = FreeRTOSMessageQueue<
        can::messages::UpdateMotorPositionEstimationRequest>;

    /**
     * Construct a motor
     *
     * @param lms_config Linear motion system configuration
     * @param hardware_iface Hardware interface
     * @param constraints Motion constraints
     * @param queue Input message queue containing motion commands.
     * @param update_queue Message queue to the Interrupt Handler containing
     *                     UpdateMotorPositionEstimationRequest messages
     * commands.
     */
    Motor(lms::LinearMotionSystemConfig<MEConfig> lms_config,
          motor_hardware::StepperMotorHardwareIface& hardware_iface,
          MotionConstraints constraints, GenericQueue& queue,
          UpdatePositionQueue& update_queue, bool engage_on_boot = false)

        : pending_move_queue(queue), motion_controller{lms_config,
                                                       hardware_iface,
                                                       constraints,
                                                       pending_move_queue,
                                                       update_queue,
                                                       engage_on_boot} {}
    GenericQueue& pending_move_queue;
    motion_controller::MotionController<MEConfig> motion_controller;

    Motor(const Motor&) = delete;
    auto operator=(const Motor&) -> Motor& = delete;
    Motor(Motor&&) = delete;
    auto operator=(Motor&&) -> Motor&& = delete;
    ~Motor() = default;
};

}  // namespace motor_class
