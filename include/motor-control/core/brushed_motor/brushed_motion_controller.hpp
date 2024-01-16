#pragma once

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/brushed_motor/error_tolerance_config.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
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
        BrushedMove msg{
            .message_index = can_msg.message_index,
            .duration = can_msg.duration,
            .duty_cycle = can_msg.duty_cycle,
            .group_id = can_msg.group_id,
            .seq_id = can_msg.seq_id,
            .stay_engaged = can_msg.stay_engaged,
            .stop_condition = MoveStopCondition::none,
            .usage_key = hardware.get_usage_eeprom_config().get_distance_key()};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    void move(const can::messages::GripperHomeRequest& can_msg) {
        BrushedMove msg{
            .message_index = can_msg.message_index,
            .duration = can_msg.duration,
            .duty_cycle = can_msg.duty_cycle,
            .group_id = can_msg.group_id,
            .seq_id = can_msg.seq_id,
            .stop_condition = MoveStopCondition::limit_switch,
            .usage_key = hardware.get_usage_eeprom_config().get_distance_key()};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    void move(const can::messages::AddBrushedLinearMoveRequest& can_msg) {
        BrushedMove msg{
            .message_index = can_msg.message_index,
            .duration = can_msg.duration,
            .duty_cycle = 0UL,
            .group_id = can_msg.group_id,
            .seq_id = can_msg.seq_id,
            .encoder_position =
                int32_t(can_msg.encoder_position_um /
                        get_mechanical_config().get_encoder_um_per_pulse()),
            .stop_condition = MoveStopCondition::encoder_position,
            .usage_key = hardware.get_usage_eeprom_config().get_distance_key()};
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

    void stop(
        can::ids::ErrorSeverity error_severity =
            can::ids::ErrorSeverity::warning,
        can::ids::ErrorCode error_code = can::ids::ErrorCode::stop_requested) {
        queue.reset();
        // if we're gripping something we need to flag this so we don't drop it
        if (!hardware.get_stay_enabled()) {
            if (hardware.is_timer_interrupt_running()) {
                hardware.set_cancel_request(error_severity, error_code);
            }
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

    void set_idle_holdoff(
        const can::messages::SetGripperJawHoldoffRequest& can_msg) {
        error_config.update_idle_holdoff_ticks(
            fixed_point_to_float(can_msg.holdoff_ms, S15Q16_RADIX));
    }

    auto get_idle_holdoff_ms() -> uint32_t {
        return convert_to_fixed_point(error_config.get_holdoff_ms(),
                                      S15Q16_RADIX);
    }

    template <usage_storage_task::TaskClient UsageClient>
    void send_usage_data(uint32_t message_index, UsageClient& usage_client) {
        usage_messages::GetUsageRequest req = {
            .message_index = message_index,
            .usage_conf = hardware.get_usage_eeprom_config()};
        usage_client.send_usage_storage_queue(req);
    }

    auto get_jaw_state() -> BrushedMotorState {
        return hardware.get_motor_state();
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
