#pragma once

#include <type_traits>
#include <variant>

#include "can/core/messages.hpp"
#include "common/firmware/motor.h"
#include "common/firmware/timer_interrupt.h"
#include "linear_motion_system.hpp"
#include "motor_messages.hpp"
#include "step_motor.hpp"

namespace motion_controller {

using namespace motor_messages;

struct HardwareConfig {
    struct PinConfig direction;
    struct PinConfig step;
    struct PinConfig enable;
};
/*
 * MotionController is responsible for motor movement and communicate with
 * the motor driver using the HAL driver API and SPI.
 */
template <template <class> class QueueImpl,
          template <class> class CompletedQueueImpl,
          lms::MotorMechanicalConfig MEConfig>
requires MessageQueue<QueueImpl<Move>, Move> &&
    MessageQueue<CompletedQueueImpl<Ack>, Ack>
class MotionController {
  public:
    using GenericQueue = QueueImpl<Move>;
    using CompletedQueue = CompletedQueueImpl<Ack>;
    MotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                     HardwareConfig& config, GenericQueue& queue,
                     CompletedQueue& completed_queue)
        : linear_motion_sys_config(lms_config),
          hardware_config(config),
          queue(queue),
          completed_queue(completed_queue) {
        timer_init();
        setup();
    }

    MotionController& operator=(MotionController&) = delete;
    MotionController&& operator=(MotionController&&) = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;

    ~MotionController() {
        reset_pin(hardware_config.enable);
        timer_interrupt_stop();
    }

    void setup() {
        start_motor_handler(&queue, &completed_queue);
        timer_interrupt_start();
        // convert steps to mm to fixed point here
        steps_per_mm =
            static_cast<uint32_t>(linear_motion_sys_config.get_steps_per_mm());
    }

    void move(const can_messages::MoveRequest& can_msg) {
        Move msg{
            .duration = can_msg.duration,
            .velocity = can_msg.velocity,
            .acceleration = can_msg.acceleration,
            .group_id = NO_GROUP,
            .seq_id = NO_GROUP,
        };
        queue.try_write(msg);
    }

    void move(const can_messages::AddLinearMoveRequest& can_msg) {
        Move msg{.duration = can_msg.duration,
                 .velocity = can_msg.velocity,
                 .acceleration = can_msg.acceleration,
                 .group_id = can_msg.group_id,
                 .seq_id = can_msg.seq_id};
        queue.try_write(msg);
    }

    void enable_motor() { set_pin(hardware_config.enable); }

    void disable_motor() { reset_pin(hardware_config.enable); }

    void stop() {
        reset_pin(hardware_config.step);
        timer_interrupt_stop();
    }

    auto get_direction() -> bool { return direction; }

  private:
    bool direction = true;  // direction true: forward, false: backward
    uint32_t steps_per_mm;
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    HardwareConfig& hardware_config;
    GenericQueue& queue;
    CompletedQueue& completed_queue;
};

}  // namespace motion_controller
