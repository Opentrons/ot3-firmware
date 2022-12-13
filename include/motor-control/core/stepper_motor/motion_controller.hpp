#pragma once

#include <type_traits>
#include <variant>

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/types.hpp"
#include "motor-control/core/utils.hpp"

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
    using UpdatePositionQueue = freertos_message_queue::FreeRTOSMessageQueue<
        can::messages::UpdateMotorPositionRequest>;
    MotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                     StepperMotorHardwareIface& hardware_iface,
                     MotionConstraints constraints, GenericQueue& queue,
                     UpdatePositionQueue& update_queue,
                     bool eng_on_strt = false)
        : linear_motion_sys_config(lms_config),
          hardware(hardware_iface),
          motion_constraints(constraints),
          queue(queue),
          update_queue(update_queue),
          steps_per_mm(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_steps_per_mm(), 31)),
          um_per_step(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_um_per_step(), 31)),
          um_per_encoder_pulse(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_encoder_um_per_pulse(), 31)),
          engage_at_startup(eng_on_strt) {}

    auto operator=(const MotionController&) -> MotionController& = delete;
    auto operator=(MotionController&&) -> MotionController&& = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;

    ~MotionController() = default;

    [[nodiscard]] auto get_mechanical_config() const
        -> const lms::LinearMotionSystemConfig<MEConfig>& {
        return linear_motion_sys_config;
    }

    void move(const can::messages::AddLinearMoveRequest& can_msg) {
        steps_per_tick velocity_steps =
            fixed_point_multiply(steps_per_mm, can_msg.velocity);
        steps_per_tick_sq acceleration_steps =
            fixed_point_multiply(steps_per_mm, can_msg.acceleration);
        Move msg{.message_index = can_msg.message_index,
                 .duration = can_msg.duration,
                 .velocity = velocity_steps,
                 .acceleration = acceleration_steps,
                 .group_id = can_msg.group_id,
                 .seq_id = can_msg.seq_id,
                 .stop_condition = static_cast<MoveStopCondition>(
                     can_msg.request_stop_condition)};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    void move(const can::messages::HomeRequest& can_msg) {
        steps_per_tick velocity_steps =
            fixed_point_multiply(steps_per_mm, can_msg.velocity);
        Move msg{.message_index = can_msg.message_index,
                 .duration = can_msg.duration,
                 .velocity = velocity_steps,
                 .acceleration = 0,
                 .group_id = can_msg.group_id,
                 .seq_id = can_msg.seq_id,
                 .stop_condition = MoveStopCondition::limit_switch};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    [[nodiscard]] auto update_position(
        const can::messages::UpdateMotorPositionRequest& can_msg) -> bool {
        if (!enabled) {
            return false;
        }
        return update_queue.try_write(can_msg);
    }

    void stop() { hardware.stop_timer_interrupt(); }

    auto read_limit_switch() -> bool { return hardware.check_limit_switch(); }

    [[nodiscard]] auto read_motor_position() const {
        return fixed_point_multiply(um_per_step, hardware.get_step_tracker());
    }

    auto read_encoder_pulses() {
        return fixed_point_multiply(um_per_encoder_pulse,
                                    hardware.get_encoder_pulses(),
                                    radix_offset_0{});
    }

    auto check_read_sync_line() -> bool { return hardware.check_sync_in(); }

    void enable_motor() {
        hardware.start_timer_interrupt();
        hardware.activate_motor();
        enabled = true;
    }

    void disable_motor() {
        hardware.deactivate_motor();
        hardware.position_flags.clear_flag(
            can::ids::MotorPositionFlags::stepper_position_ok);
        enabled = false;
    }

    void set_motion_constraints(
        const can::messages::SetMotionConstraints& can_msg) {
        motion_constraints =
            MotionConstraints{.min_velocity = can_msg.min_velocity,
                              .max_velocity = can_msg.max_velocity,
                              .min_acceleration = can_msg.min_acceleration,
                              .max_acceleration = can_msg.max_acceleration};
    }

    [[nodiscard]] auto get_motion_constraints() -> MotionConstraints {
        return motion_constraints;
    }

    [[nodiscard]] auto get_position_flags() const -> uint8_t {
        return hardware.position_flags.get_flags();
    }

  private:
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    StepperMotorHardwareIface& hardware;
    MotionConstraints motion_constraints;
    GenericQueue& queue;
    UpdatePositionQueue& update_queue;
    sq31_31 steps_per_mm{0};
    sq31_31 um_per_step{0};
    sq31_31 um_per_encoder_pulse{0};
    bool enabled = false;

  public:
    bool engage_at_startup;
};

}  // namespace motion_controller

namespace pipette_motion_controller {

using namespace motor_messages;
using namespace motor_hardware;

// TODO(lc 05-22-2022) We should have PipetteMotionController inherit
// from MotionController, but that will require some template refactoring
// to make it compile.
template <lms::MotorMechanicalConfig MEConfig>
class PipetteMotionController {
  public:
    using GenericQueue =
        freertos_message_queue::FreeRTOSMessageQueue<GearMotorMove>;
    PipetteMotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                            PipetteStepperMotorHardwareIface& hardware_iface,
                            MotionConstraints constraints, GenericQueue& queue)
        : linear_motion_sys_config(lms_config),
          hardware(hardware_iface),
          motion_constraints(constraints),
          queue(queue),
          steps_per_mm(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_steps_per_mm(), 31)),
          um_per_step(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_um_per_step(), 31)) {}

    auto operator=(const PipetteMotionController&)
        -> PipetteMotionController& = delete;
    auto operator=(PipetteMotionController&&)
        -> PipetteMotionController&& = delete;
    PipetteMotionController(PipetteMotionController&) = delete;
    PipetteMotionController(PipetteMotionController&&) = delete;

    ~PipetteMotionController() = default;

    [[nodiscard]] auto get_mechanical_config() const
        -> const lms::LinearMotionSystemConfig<MEConfig>& {
        return linear_motion_sys_config;
    }

    void move(const can::messages::TipActionRequest& can_msg) {
        steps_per_tick velocity_steps =
            fixed_point_multiply(steps_per_mm, can_msg.velocity);
        GearMotorMove msg{
            can_msg.message_index,
            can_msg.duration,
            velocity_steps,
            0,
            can_msg.group_id,
            can_msg.seq_id,
            static_cast<MoveStopCondition>(can_msg.request_stop_condition),
            can_msg.action};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    [[nodiscard]] auto update_position(
        const can::messages::UpdateMotorPositionRequest&) -> bool {
        // Not supported for gear motors - no encoder!
        return false;
    }

    void stop() { hardware.stop_timer_interrupt(); }

    auto read_limit_switch() -> bool { return hardware.check_limit_switch(); }

    auto check_read_sync_line() -> bool { return hardware.check_sync_in(); }

    auto check_tip_sense() -> bool { return hardware.check_tip_sense(); }

    void enable_motor() {
        hardware.start_timer_interrupt();
        hardware.activate_motor();
        enabled = true;
    }

    void disable_motor() {
        hardware.deactivate_motor();
        hardware.position_flags.clear_flag(
            can::ids::MotorPositionFlags::stepper_position_ok);
        enabled = false;
    }

    void set_motion_constraints(
        const can::messages::SetMotionConstraints& can_msg) {
        motion_constraints =
            MotionConstraints{.min_velocity = can_msg.min_velocity,
                              .max_velocity = can_msg.max_velocity,
                              .min_acceleration = can_msg.min_acceleration,
                              .max_acceleration = can_msg.max_acceleration};
    }

    [[nodiscard]] auto read_motor_position() const {
        return fixed_point_multiply(um_per_step, hardware.get_step_tracker());
    }

    [[nodiscard]] auto get_motion_constraints() -> MotionConstraints {
        return motion_constraints;
    }

    [[nodiscard]] auto get_position_flags() const -> uint8_t {
        return hardware.position_flags.get_flags();
    }

  private:
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    PipetteStepperMotorHardwareIface& hardware;
    MotionConstraints motion_constraints;
    GenericQueue& queue;
    sq31_31 steps_per_mm{0};
    sq31_31 um_per_step{0};
    bool enabled = false;
};

}  // namespace pipette_motion_controller
