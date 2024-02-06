#pragma once

#include <type_traits>
#include <variant>

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
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
    using GenericQueue =
        freertos_message_queue::FreeRTOSMessageQueue<SensorSyncMove>;
    using UpdatePositionQueue = freertos_message_queue::FreeRTOSMessageQueue<
        can::messages::UpdateMotorPositionEstimationRequest>;
    MotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                     StepperMotorHardwareIface& hardware_iface,
                     MotionConstraints constraints, GenericQueue& queue,
                     UpdatePositionQueue& update_queue,
                     bool diseng_on_strt = false)
        : linear_motion_sys_config(lms_config),
          hardware(hardware_iface),
          motion_constraints(constraints),
          queue(queue),
          update_queue(update_queue),
          steps_per_mm(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_usteps_per_mm(), 31)),
          steps_per_um(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_usteps_per_um(), 31)),
          um_per_step(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_um_per_step(), 31)),
          um_per_encoder_pulse(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_encoder_um_per_pulse(), 31)),
          disengage_at_startup(diseng_on_strt) {}

    auto operator=(const MotionController&) -> MotionController& = delete;
    auto operator=(MotionController&&) -> MotionController&& = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;

    ~MotionController() = default;

    [[nodiscard]] auto get_mechanical_config() const
        -> const lms::LinearMotionSystemConfig<MEConfig>& {
        return linear_motion_sys_config;
    }

    void move(const can::messages::AddSensorMoveRequest& can_msg) {
        steps_per_tick velocity_steps =
            fixed_point_multiply(steps_per_mm, can_msg.velocity);
        steps_per_tick_sq acceleration_steps =
            fixed_point_multiply(steps_per_um, can_msg.acceleration);
        SensorSyncMove msg{
            can_msg.message_index,
            can_msg.duration,
            velocity_steps,
            acceleration_steps,
            can_msg.group_id,
            can_msg.seq_id,
            can_msg.request_stop_condition,
            0,
            hardware.get_usage_eeprom_config().get_distance_key(),
            can_msg.sensor_id};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    void move(const can::messages::AddLinearMoveRequest& can_msg) {
        steps_per_tick velocity_steps =
            fixed_point_multiply(steps_per_mm, can_msg.velocity);
        steps_per_tick_sq acceleration_steps =
            fixed_point_multiply(steps_per_um, can_msg.acceleration);
        SensorSyncMove msg{
            .message_index = can_msg.message_index,
            .duration = can_msg.duration,
            .velocity = velocity_steps,
            .acceleration = acceleration_steps,
            .group_id = can_msg.group_id,
            .seq_id = can_msg.seq_id,
            .stop_condition = can_msg.request_stop_condition,
            .usage_key = hardware.get_usage_eeprom_config().get_distance_key(),
            .sensor_id = can::ids::SensorId::UNUSED};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    void move(const can::messages::HomeRequest& can_msg) {
        steps_per_tick velocity_steps =
            fixed_point_multiply(steps_per_mm, can_msg.velocity);
        SensorSyncMove msg{
            .message_index = can_msg.message_index,
            .duration = can_msg.duration,
            .velocity = velocity_steps,
            .acceleration = 0,
            .group_id = can_msg.group_id,
            .seq_id = can_msg.seq_id,
            .stop_condition =
                static_cast<uint8_t>(MoveStopCondition::limit_switch),
            .usage_key = hardware.get_usage_eeprom_config().get_distance_key(),
            .sensor_id = can::ids::SensorId::UNUSED};
        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    [[nodiscard]] auto update_position(
        const can::messages::UpdateMotorPositionEstimationRequest& can_msg)
        -> bool {
        if (!enabled) {
            return false;
        }
        return update_queue.try_write(can_msg);
    }

    void stop() {
        queue.reset();
        if (hardware.is_timer_interrupt_running()) {
            hardware.request_cancel();
        }
    }

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

    template <usage_storage_task::TaskClient UsageClient>
    void send_usage_data(uint32_t message_index, UsageClient& usage_client) {
        usage_messages::GetUsageRequest req = {
            .message_index = message_index,
            .usage_conf = hardware.get_usage_eeprom_config()};
        usage_client.send_usage_storage_queue(req);
    }

  private:
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    StepperMotorHardwareIface& hardware;
    MotionConstraints motion_constraints;
    GenericQueue& queue;
    UpdatePositionQueue& update_queue;
    sq31_31 steps_per_mm{0};
    sq31_31 steps_per_um{0};
    sq31_31 um_per_step{0};
    sq31_31 um_per_encoder_pulse{0};
    bool enabled = false;

  public:
    bool disengage_at_startup;
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
                            StepperMotorHardwareIface& hardware_iface,
                            MotionConstraints constraints, GenericQueue& queue,
                            can::ids::GearMotorId gear_motor_id)
        : linear_motion_sys_config(lms_config),
          hardware(hardware_iface),
          motion_constraints(constraints),
          queue(queue),
          steps_per_um(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_usteps_per_um(), 31)),
          steps_per_mm(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_usteps_per_mm(), 31)),
          um_per_step(convert_to_fixed_point_64_bit(
              linear_motion_sys_config.get_um_per_step(), 31)),
          gear_motor_id(gear_motor_id) {}

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
        steps_per_tick_sq acceleration_steps =
            fixed_point_multiply(steps_per_um, can_msg.acceleration);
        GearMotorMove msg{
            can_msg.message_index,
            can_msg.duration,
            velocity_steps,
            acceleration_steps,
            can_msg.group_id,
            can_msg.seq_id,
            can_msg.request_stop_condition,
            0,
            hardware.get_usage_eeprom_config().get_gear_distance_key(),
            hardware.get_step_tracker(),
            can_msg.action,
            gear_motor_id,
            can::ids::SensorId::UNUSED};

        if (!enabled) {
            enable_motor();
        }
        queue.try_write(msg);
    }

    [[nodiscard]] auto update_position(
        const can::messages::UpdateMotorPositionEstimationRequest&) -> bool {
        // Not supported for gear motors - no encoder!
        return false;
    }

    void stop() {
        queue.reset();
        // if the timer interrupt is running, cancel it. if it isn't running,
        // don't submit a cancel because then the cancel won't be read until
        // the timer starts the next time.
        if (hardware.is_timer_interrupt_running()) {
            hardware.request_cancel();
        }
    }

    auto read_limit_switch() -> bool { return hardware.check_limit_switch(); }

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

    [[nodiscard]] auto read_motor_position() const {
        return fixed_point_multiply(um_per_step, hardware.get_step_tracker());
    }

    [[nodiscard]] auto get_motion_constraints() -> MotionConstraints {
        return motion_constraints;
    }

    [[nodiscard]] auto get_position_flags() const -> uint8_t {
        return hardware.position_flags.get_flags();
    }

    template <usage_storage_task::TaskClient UsageClient>
    void send_usage_data(uint32_t message_index, UsageClient& usage_client) {
        usage_messages::GetUsageRequest req = {
            .message_index = message_index,
            .usage_conf = hardware.get_usage_eeprom_config()};
        usage_client.send_usage_storage_queue(req);
    }

  private:
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    StepperMotorHardwareIface& hardware;
    MotionConstraints motion_constraints;
    GenericQueue& queue;

    sq31_31 steps_per_um{0};
    sq31_31 steps_per_mm{0};
    sq31_31 um_per_step{0};
    bool enabled = false;
    can::ids::GearMotorId gear_motor_id;
};

}  // namespace pipette_motion_controller
