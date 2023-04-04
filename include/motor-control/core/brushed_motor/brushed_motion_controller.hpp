#pragma once

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/brushed_motor/error_tolerance_config.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/utils.hpp"

namespace brushed_motion_controller {

using namespace motor_hardware;
using namespace motor_messages;
using namespace error_tolerance_config;

template <lms::MotorMechanicalConfig MEConfig>
class MotionController {
  public:
    using GenericQueue =
        freertos_message_queue::FreeRTOSMessageQueue<BrushedMove>;
    MotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                     BrushedMotorHardwareIface& hardware_iface,
                     GenericQueue& queue,
                     BrushedMotorErrorTolerance& error_conf)
        : linear_motion_sys_config(lms_config),
          hardware(hardware_iface),
          queue(queue),
          um_per_encoder_pulse(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_encoder_um_per_pulse(), 31)),
          error_config(error_conf) {}

    auto operator=(const MotionController&) -> MotionController& = delete;
    auto operator=(MotionController&&) -> MotionController&& = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;
    ~MotionController() = default;

    [[nodiscard]] auto get_mechanical_config() const
        -> const lms::LinearMotionSystemConfig<MEConfig>& {
        return linear_motion_sys_config;
    }

    void move(const can::messages::GripperGripRequest& can_msg) {
        BrushedMove msg{.message_index = can_msg.message_index,
                        .duration = can_msg.duration,
                        .duty_cycle = can_msg.duty_cycle,
                        .group_id = can_msg.group_id,
                        .seq_id = can_msg.seq_id,
                        .stop_condition = MoveStopCondition::none,
                        .start_encoder_position = hardware.get_encoder_pulses()};
        enable_motor();
        queue.try_write(msg);
    }

    void move(const can::messages::GripperHomeRequest& can_msg) {
        BrushedMove msg{.message_index = can_msg.message_index,
                        .duration = can_msg.duration,
                        .duty_cycle = can_msg.duty_cycle,
                        .group_id = can_msg.group_id,
                        .seq_id = can_msg.seq_id,
                        .stop_condition = MoveStopCondition::limit_switch,
                        .start_encoder_position = hardware.get_encoder_pulses()};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    void move(const can::messages::AddBrushedLinearMoveRequest& can_msg) {
        BrushedMove msg{.message_index = can_msg.message_index,
                        .duration = can_msg.duration,
                        .duty_cycle = 0UL,
                        .group_id = can_msg.group_id,
                        .seq_id = can_msg.seq_id,
                        .encoder_position = int32_t(
                            can_msg.encoder_position_um /
                            get_mechanical_config().get_encoder_um_per_pulse()),
                        .stop_condition = MoveStopCondition::encoder_position,
                        .start_encoder_position = hardware.get_encoder_pulses()};
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
        enabled = false;
    }

    void stop() {
        queue.reset();
        // if we're gripping something we need to flag this so we don't drop it
        if (!hardware.get_stay_enabled()) {
            disable_motor();
        }
    }

    auto read_limit_switch() -> bool { return hardware.check_limit_switch(); }

    auto read_encoder_pulses() {
        return fixed_point_multiply(um_per_encoder_pulse,
                                    hardware.get_encoder_pulses(),
                                    radix_offset_0{});
    }

    [[nodiscard]] auto get_position_flags() const -> uint8_t {
        return hardware.position_flags.get_flags();
    }

    void set_error_tolerance(
        const can::messages::SetGripperErrorToleranceRequest& can_msg) {
        error_config.update_tolerance(
            fixed_point_to_float(can_msg.max_pos_error_mm, S15Q16_RADIX),
            fixed_point_to_float(can_msg.max_unwanted_movement_mm,
                                 S15Q16_RADIX));
    }

  private:
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    BrushedMotorHardwareIface& hardware;
    GenericQueue& queue;
    bool enabled = false;
    sq31_31 um_per_encoder_pulse{0};
    BrushedMotorErrorTolerance& error_config;
};

}  // namespace brushed_motion_controller
