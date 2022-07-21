#pragma once

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/utils.hpp"

namespace brushed_motion_controller {

using namespace motor_hardware;
using namespace motor_messages;

template <lms::MotorMechanicalConfig MEConfig>
class MotionController {
  public:
    using GenericQueue =
        freertos_message_queue::FreeRTOSMessageQueue<BrushedMove>;
    MotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                     BrushedMotorHardwareIface& hardware_iface,
                     GenericQueue& queue)
        : linear_motion_sys_config(lms_config),
          hardware(hardware_iface),
          queue(queue),
          um_per_encoder_pulse(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_encoder_um_per_pulse(), 31)) {}

    auto operator=(const MotionController&) -> MotionController& = delete;
    auto operator=(MotionController&&) -> MotionController&& = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;
    ~MotionController() = default;

    void move(const can::messages::GripperGripRequest& can_msg) {
        BrushedMove msg{.duration = can_msg.duration,
                        .duty_cycle = can_msg.duty_cycle,
                        .group_id = can_msg.group_id,
                        .seq_id = can_msg.seq_id,
                        .stop_condition = MoveStopCondition::none};
        enable_motor();
        queue.try_write(msg);
    }

    void move(const can::messages::GripperHomeRequest& can_msg) {
        BrushedMove msg{.duration = can_msg.duration,
                        .duty_cycle = can_msg.duty_cycle,
                        .group_id = can_msg.group_id,
                        .seq_id = can_msg.seq_id,
                        .stop_condition = MoveStopCondition::limit_switch};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    void enable_motor() {
        hardware.activate_motor();
        hardware.start_timer_interrupt();
        enabled = true;
    }

    void disable_motor() {
        hardware.deactivate_motor();
        hardware.stop_timer_interrupt();
        enabled = false;
    }

    void stop() { hardware.stop_pwm(); }

    auto read_limit_switch() -> bool { return hardware.check_limit_switch(); }

    auto read_encoder_pulses() {
        return fixed_point_multiply(um_per_encoder_pulse,
                                    hardware.get_encoder_pulses(),
                                    radix_offset_0{});
    }

  private:
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    BrushedMotorHardwareIface& hardware;
    GenericQueue& queue;
    bool enabled = false;
    sq31_31 um_per_encoder_pulse{0};
};

}  // namespace brushed_motion_controller
