#pragma once

#include <algorithm>
#include <atomic>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "ot_utils/core/pid.hpp"

namespace brushed_motor_handler {

using namespace motor_messages;

/*
 *
 * A brushed motor motion handler class.
 *
 */

enum class ControlState {
    FORCE_CONTROLLING_HOME,
    FORCE_CONTROLLING,
    POSITION_CONTROLLING,
    IDLE,
    ACTIVE,
    ERROR,
    ESTOP
};

static constexpr uint32_t HOLDOFF_TICKS =
    32;  // hold off for 1 ms (with a 32k Hz timer)
// using the logic analyzer it takes about 0.2-0.3 ms for the output
// to stablize after changing directions of the PWM

// upon advice from hardware, 0.01mm is a good limit for precision
static constexpr double ACCEPTABLE_DISTANCE_TOLERANCE_MM = 0.01;
static constexpr double UNWANTED_MOVEMENT_DISTANCE_MM = 0.02;

template <template <class> class QueueImpl,
          move_status_reporter_task::BrushedTaskClient StatusClient>
requires MessageQueue<QueueImpl<BrushedMove>, BrushedMove>
class BrushedMotorInterruptHandler {
  public:
    using GenericQueue = QueueImpl<BrushedMove>;

    BrushedMotorInterruptHandler() = delete;
    BrushedMotorInterruptHandler(
        GenericQueue& incoming_queue, StatusClient& outgoing_queue,
        motor_hardware::BrushedMotorHardwareIface& hardware_iface,
        brushed_motor_driver::BrushedMotorDriverIface& driver_iface,
        lms::LinearMotionSystemConfig<lms::GearBoxConfig>& gearbox_config)
        : queue(incoming_queue),
          status_queue_client(outgoing_queue),
          hardware(hardware_iface),
          driver_hardware(driver_iface),
          gear_conf(gearbox_config) {
        acceptable_position_error =
            int32_t(gear_conf.get_encoder_pulses_per_mm() *
                    ACCEPTABLE_DISTANCE_TOLERANCE_MM);
        unwanted_movement_threshold =
            int32_t(gear_conf.get_encoder_pulses_per_mm() *
                    UNWANTED_MOVEMENT_DISTANCE_MM);
    }
    ~BrushedMotorInterruptHandler() = default;
    auto operator=(BrushedMotorInterruptHandler&)
        -> BrushedMotorInterruptHandler& = delete;
    auto operator=(BrushedMotorInterruptHandler&&)
        -> BrushedMotorInterruptHandler&& = delete;
    BrushedMotorInterruptHandler(BrushedMotorInterruptHandler&) = delete;
    BrushedMotorInterruptHandler(BrushedMotorInterruptHandler&&) = delete;

    [[nodiscard]] auto has_messages() const -> bool {
        return queue.has_message_isr();
    }

    void controlled_move_to(int32_t move_delta) {
        uint32_t old_control_pwm = current_control_pwm;
        // pass through early if we're already within acceptable position
        if (std::abs(move_delta) < acceptable_position_error) {
            current_control_pwm = 0;
        } else {
            if (move_delta < 0) {
                tick = 0;
                hardware.grip();
            } else if (move_delta > 0) {
                tick = 0;
                hardware.ungrip();
            }
            double pid_output = hardware.update_control(move_delta);
            // The min and max PWM values for an active duty move are hardware
            // dependent so let the driver decide the bounds of the pid output
            // take the abs of the pid output because the pwm value has to be
            // always positive
            current_control_pwm = driver_hardware.pwm_active_duty_clamp(
                int(std::abs(pid_output)));
        }
        if (old_control_pwm != current_control_pwm) {
            driver_hardware.update_pwm_settings(current_control_pwm);
        }
    }

    void execute_active_move() {
        int32_t move_delta = 0;
        switch (buffered_move.stop_condition) {
            // homing move
            case MoveStopCondition::limit_switch:
                // We double check for is_idle here because there was a POC
                // gripper with a noisy limit switch that triggered prematurely
                // this just adds a double check and we can use it later to
                // trigger an error if is_idle is true but the limit switch
                // isn't triggered
                if (limit_switch_triggered() && is_idle) {
                    homing_stopped();
                }
                break;
            // linear move
            case MoveStopCondition::encoder_position:
                move_delta = hardware.get_encoder_pulses() -
                             buffered_move.encoder_position;
                controlled_move_to(move_delta);
                if (std::abs(move_delta) < acceptable_position_error) {
                    finish_current_move(AckMessageId::stopped_by_condition);
                }
                break;
            // grip move
            case MoveStopCondition::none:
            case MoveStopCondition::gripper_force:
            case MoveStopCondition::stall:
                if (is_sensing() && is_idle) {
                    finish_current_move(
                        AckMessageId::complete_without_condition);
                }
                break;
            case MoveStopCondition::cap_sensor:
                // TODO write cap sensor move code
                break;
        }
    }

    void execute_idle_move() {
        if (has_messages()) {
            update_and_start_move();
        } else if (motor_state == ControlState::POSITION_CONTROLLING) {
            int32_t move_delta =
                hardware.get_encoder_pulses() - hold_encoder_position;
            controlled_move_to(move_delta);
            // we use double the acceptable position here just to allow the pid
            // loop the opportunity to maintain small movements that occur from
            // motion and vibration
            if (move_delta > unwanted_movement_threshold) {
                cancel_and_clear_moves(can::ids::ErrorCode::collision_detected);
            }
        } else if (motor_state == ControlState::FORCE_CONTROLLING ||
                   motor_state == ControlState::FORCE_CONTROLLING_HOME) {
            if (!is_idle &&
                std::abs(hardware.get_encoder_pulses() -
                         hold_encoder_position) > unwanted_movement_threshold) {
                // we have likely dropped a labware or had a collision
                auto err = motor_state == ControlState::FORCE_CONTROLLING
                               ? can::ids::ErrorCode::labware_dropped
                               : can::ids::ErrorCode::collision_detected;
                cancel_and_clear_moves(err);
            }
        }
    }

    void run_interrupt() {
        if (motor_state == ControlState::ESTOP) {
            // return out of error state once the estop is disabled
            if (!estop_triggered()) {
                motor_state = ControlState::IDLE;
                status_queue_client.send_brushed_move_status_reporter_queue(
                    can::messages::ErrorMessage{
                        .message_index = 0,
                        .severity = can::ids::ErrorSeverity::warning,
                        .error_code = can::ids::ErrorCode::estop_released});
            }
        } else {
            if (estop_triggered()) {
                cancel_and_clear_moves(can::ids::ErrorCode::estop_detected);
                return;
            }
            if (tick < HOLDOFF_TICKS) {
                tick++;
                return;
            }
            if (motor_state == ControlState::ACTIVE) {
                execute_active_move();
            } else {
                execute_idle_move();
            }
        }
    }

    auto is_sensing() -> bool {
        /* When gripping, hold off a certain number of ticks before checking if
         * the encoder is idle */
        return !(buffered_move.stop_condition ==
                     MoveStopCondition::limit_switch ||
                 tick < HOLDOFF_TICKS);
    }

    void set_enc_idle_state(bool val) {
        /* When the encoder speed timer overflows, this means the encoder is
         * moving at a frequency lower than 8 Hz, we can assume the gripper jaw
         * has stopped moving and the active move is complete.
         */
        is_idle = val;
    }

    void update_and_start_move() {
        if (queue.try_read_isr(&buffered_move)) {
            motor_state = ControlState::ACTIVE;
        }
        if (buffered_move.duty_cycle != 0U) {
            driver_hardware.update_pwm_settings(buffered_move.duty_cycle);
        }
        // clear the old states
        hardware.reset_control();
        hardware.set_stay_enabled(false);

        switch (buffered_move.stop_condition) {
            case MoveStopCondition::limit_switch:
                tick = 0;
                hardware.ungrip();
                break;
            case MoveStopCondition::encoder_position:
                if (hardware.get_encoder_pulses() ==
                    buffered_move.encoder_position) {
                    LOG("Attempting to add a gripper move to our current "
                        "position");
                    finish_current_move(AckMessageId::stopped_by_condition);
                }
                break;
            case MoveStopCondition::none:
            case MoveStopCondition::gripper_force:
            case MoveStopCondition::stall:
                tick = 0;
                hardware.grip();
                break;
            case MoveStopCondition::cap_sensor:
                // this is an unused move stop condition for the brushed motor
                // just return with no condition
                // TODO creat can bus error messages and send that instead
                finish_current_move(AckMessageId::complete_without_condition);
                break;
        }
    }

    void homing_stopped() {
        hardware.reset_encoder_pulses();
        finish_current_move(AckMessageId::stopped_by_condition);
    }

    auto limit_switch_triggered() -> bool {
        return hardware.check_limit_switch();
    }

    auto estop_triggered() -> bool { return hardware.check_estop_in(); }

    void cancel_and_clear_moves(
        can::ids::ErrorCode err_code = can::ids::ErrorCode::hardware) {
        // If there is a currently running move send a error corresponding
        // to it so the hardware controller can know what move was running
        // when the cancel happened
        uint32_t message_index = 0;
        if (motor_state == ControlState::ACTIVE) {
            message_index = buffered_move.message_index;
        }

        // if we think we dropped a labware we don't want the controller
        // to stop the motor in case we only slipped or collided and still
        // have the labware in the jaws
        if (err_code == can::ids::ErrorCode::labware_dropped) {
            hardware.set_stay_enabled(true);
        }

        status_queue_client.send_brushed_move_status_reporter_queue(
            can::messages::ErrorMessage{
                .message_index = message_index,
                .severity = can::ids::ErrorSeverity::unrecoverable,
                .error_code = err_code});
        // Broadcast a stop message
        status_queue_client.send_brushed_move_status_reporter_queue(
            can::messages::StopRequest{.message_index = 0});
        if (err_code == can::ids::ErrorCode::estop_detected) {
            motor_state = ControlState::ESTOP;
        } else {
            motor_state = ControlState::ERROR;
        }
        // the queue will get reset during the stop message processing
        // we can't clear here from an interrupt context
    }

    void finish_current_move(
        AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        // if we were instructed to move to particular spot maintain that
        // position otherwise we want to continue to apply pressure
        if (buffered_move.stop_condition ==
            MoveStopCondition::encoder_position) {
            hold_encoder_position = buffered_move.encoder_position;
            motor_state = ControlState::POSITION_CONTROLLING;
            // the cap sensor move isn't valid here, so make sure if we got that
            // type of message that we just pass through and return the ack
        } else if (buffered_move.stop_condition !=
                   MoveStopCondition::cap_sensor) {
            hold_encoder_position = hardware.get_encoder_pulses();
            motor_state =
                buffered_move.stop_condition == MoveStopCondition::limit_switch
                    ? ControlState::FORCE_CONTROLLING_HOME
                    : ControlState::FORCE_CONTROLLING;
        }

        if (buffered_move.group_id != NO_GROUP) {
            auto ack = buffered_move.build_ack(hardware.get_encoder_pulses(), 0,
                                               ack_msg_id,
                                               buffered_move.message_index);

            static_cast<void>(
                status_queue_client.send_brushed_move_status_reporter_queue(
                    ack));
        }
        set_buffered_move(BrushedMove{});
    }

    void set_buffered_move(BrushedMove new_move) { buffered_move = new_move; }

    [[nodiscard]] auto get_buffered_move() const -> BrushedMove {
        return buffered_move;
    }

    std::atomic<bool> is_idle = true;
    uint32_t tick = 0;
    ControlState motor_state = ControlState::IDLE;

  private:
    GenericQueue& queue;
    StatusClient& status_queue_client;
    motor_hardware::BrushedMotorHardwareIface& hardware;
    brushed_motor_driver::BrushedMotorDriverIface& driver_hardware;
    lms::LinearMotionSystemConfig<lms::GearBoxConfig>& gear_conf;
    BrushedMove buffered_move = BrushedMove{};
    int32_t hold_encoder_position = 0;
    uint32_t current_control_pwm = 0;
    int32_t acceptable_position_error = 0;
    int32_t unwanted_movement_threshold = 0;
};
}  // namespace brushed_motor_handler
