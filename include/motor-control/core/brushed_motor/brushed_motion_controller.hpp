#pragma once

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace brushed_motion_controller {

using namespace motor_hardware;
using namespace motor_messages;

class MotionController {
  public:
    using GenericQueue =
        freertos_message_queue::FreeRTOSMessageQueue<BrushedMove>;
    MotionController(BrushedMotorHardwareIface& hardware_iface,
                     GenericQueue& queue)
        : hardware(hardware_iface), queue(queue) {}

    auto operator=(const MotionController&) -> MotionController& = delete;
    auto operator=(MotionController&&) -> MotionController&& = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;
    ~MotionController() = default;

    void move(const can::messages::GripperGripRequest& can_msg) {
        BrushedMove msg{.duration = can_msg.duration,
                        .freq = can_msg.freq,
                        .duty_cycle = can_msg.duty_cycle,
                        .group_id = can_msg.group_id,
                        .seq_id = can_msg.seq_id,
                        .stop_condition = MoveStopCondition::none};
        queue.try_write(msg);
    }

    void move(const can::messages::GripperHomeRequest& can_msg) {
        BrushedMove msg{.duration = can_msg.duration,
                        .freq = can_msg.freq,
                        .duty_cycle = can_msg.duty_cycle,
                        .group_id = can_msg.group_id,
                        .seq_id = can_msg.seq_id,
                        .stop_condition = MoveStopCondition::limit_switch};
        queue.try_write(msg);
    }

    void enable_motor() {
        hardware.activate_motor();
        hardware.start_timer_interrupt();
    }

    void disable_motor() {
        hardware.deactivate_motor();
        hardware.stop_timer_interrupt();
    }

    void stop() { hardware.stop_pwm(); }

    auto read_limit_switch() -> bool { return hardware.check_limit_switch(); }

  private:
    BrushedMotorHardwareIface& hardware;
    GenericQueue& queue;
};

}  // namespace brushed_motion_controller