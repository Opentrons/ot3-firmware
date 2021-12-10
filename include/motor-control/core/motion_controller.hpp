#pragma once

#include <type_traits>
#include <variant>

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/types.hpp"

namespace motion_controller {

using namespace motor_messages;
using namespace motor_hardware;

/*
 * MotionController is responsible for motor movement and communicate with
 * the motor driver using the HAL driver API and SPI.
 */
template <lms::MotorMechanicalConfig MEConfig>
class MotionController {
  public:
    using GenericQueue = freertos_message_queue::FreeRTOSMessageQueue<Move>;
    using CompletedQueue = freertos_message_queue::FreeRTOSMessageQueue<Ack>;
    MotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                     MotorHardwareIface& hardware_iface,
                     MotionConstraints constraints, GenericQueue& queue,
                     CompletedQueue& completed_queue)
        : linear_motion_sys_config(lms_config),
          hardware(hardware_iface),
          motion_constraints(constraints),
          queue(queue),
          completed_queue(completed_queue),
          steps_per_mm(convert_to_fixed_point(
              linear_motion_sys_config.get_steps_per_mm(), 31)) {}

    auto operator=(const MotionController&) -> MotionController& = delete;
    auto operator=(MotionController&&) -> MotionController&& = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;

    ~MotionController() = default;

    void move(const can_messages::AddLinearMoveRequest& can_msg) {
        convert_move_to_steps(can_msg) queue.try_write(msg);
    }

    void stop() { hardware.stop_timer_interrupt(); }

    void enable_motor() {
        hardware.start_timer_interrupt();
        hardware.activate_motor();
    }

    void disable_motor() { hardware.deactivate_motor(); }

    void set_motion_constraints(
        const can_messages::SetMotionConstraints& can_msg) {
        motion_constraints =
            MotionConstraints{.min_velocity = can_msg.min_velocity,
                              .max_velocity = can_msg.max_velocity,
                              .min_acceleration = can_msg.min_acceleration,
                              .max_acceleration = can_msg.max_acceleration};
    }

    [[nodiscard]] auto get_motion_constraints() -> MotionConstraints {
        return motion_constraints;
    }

  private:
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    MotorHardwareIface& hardware;
    MotionConstraints motion_constraints;
    GenericQueue& queue;
    CompletedQueue& completed_queue;
    sq0_31 steps_per_mm{0};

    Move convert_move_to_steps(
        const can_messages::AddLinearMoveRequest& can_msg) {
        steps_per_tick velocity_steps =
            fixed_point_multiply(can_msg.velocity, steps_per_mm);
        steps_per_tick_sq acceleration_steps =
            fixed_point_multiply(can_msg.acceleration, steps_per_mm);

        return Move{.duration = can_msg.duration,
                    .velocity = velocity_steps,
                    .acceleration = acceleration_steps,
                    .group_id = can_msg.group_id,
                    .seq_id = can_msg.seq_id};
    }
};

}  // namespace motion_controller
