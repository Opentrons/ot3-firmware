#pragma once

#include <algorithm>
#include <atomic>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/brushed_motor/error_tolerance_config.hpp"
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
        error_tolerance_config::BrushedMotorErrorTolerance&
            error_tolerance_conf)
        : queue(incoming_queue),
          status_queue_client(outgoing_queue),
          hardware(hardware_iface),
          driver_hardware(driver_iface),
          error_conf(error_tolerance_conf) {}
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
        if (std::abs(move_delta) < error_conf.acceptable_position_error) {
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
        if (timeout_ticks > buffered_move.duration) {
            finish_current_move(AckMessageId::timeout);
            return;
        }
        timeout_ticks++;
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
            case MoveStopCondition::encoder_position: {
                int32_t move_delta = hardware.get_encoder_pulses() -
                                     buffered_move.encoder_position;
                controlled_move_to(move_delta);
                if (std::abs(move_delta) <
                    error_conf.acceptable_position_error) {
                    finish_current_move(AckMessageId::stopped_by_condition);
                }
                break;
            }
            // grip move
            case MoveStopCondition::none:
            case MoveStopCondition::gripper_force:
            case MoveStopCondition::stall:
                if (is_sensing() && is_idle) {
                    finish_current_move(
                        AckMessageId::complete_without_condition);
                }
                break;
            case MoveStopCondition::ignore_stalls:
            case MoveStopCondition::limit_switch_backoff:
            case MoveStopCondition::sync_line:
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
            // we use a value higher than the acceptable position here to allow
            // the pid loop the opportunity to maintain small movements that
            // occur from motion and vibration
            if (move_delta > error_conf.unwanted_movement_threshold) {
                cancel_and_clear_moves(can::ids::ErrorCode::collision_detected);
                motor_state = ControlState::ERROR;
            }
        } else if (motor_state == ControlState::FORCE_CONTROLLING ||
                   motor_state == ControlState::FORCE_CONTROLLING_HOME) {
            auto pulses = hardware.get_encoder_pulses();
            if (!is_idle && pulses >= 0 &&
                std::abs(pulses - hold_encoder_position) >
                    error_conf.unwanted_movement_threshold) {
                // we have likely dropped a labware or had a collision
                auto err = motor_state == ControlState::FORCE_CONTROLLING
                               ? can::ids::ErrorCode::labware_dropped
                               : can::ids::ErrorCode::collision_detected;
                cancel_and_clear_moves(err);
                motor_state = ControlState::ERROR;
            }
        }
    }

    void run_interrupt() {
        if (clear_queue_until_empty) {
            clear_queue_until_empty = pop_and_discard_move();
        } else if (motor_state == ControlState::ESTOP) {
            // if we've received a stop request during this time we can clear
            // that flag since there is isn't anything running
            std::ignore = hardware.has_cancel_request();
            // return out of error state once the estop is disabled
            if (!estop_triggered()) {
                motor_state = ControlState::IDLE;
                status_queue_client.send_brushed_move_status_reporter_queue(
                    can::messages::ErrorMessage{
                        .message_index = 0,
                        .severity = can::ids::ErrorSeverity::warning,
                        .error_code = can::ids::ErrorCode::estop_released});
                status_queue_client.send_brushed_move_status_reporter_queue(
                    usage_messages::IncreaseErrorCount{
                        .key = hardware.get_usage_eeprom_config()
                                   .get_error_count_key()});
            }
        } else if (estop_triggered()) {
            cancel_and_clear_moves(can::ids::ErrorCode::estop_detected);
            motor_state = ControlState::ESTOP;
        } else if (hardware.has_cancel_request()) {
            cancel_and_clear_moves(can::ids::ErrorCode::stop_requested, can::ids::ErrorSeverity::warning);
            motor_state = ControlState::IDLE;
        } else if (tick < HOLDOFF_TICKS) {
            tick++;
        } else if (motor_state == ControlState::ACTIVE) {
            execute_active_move();
        } else {
            execute_idle_move();
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

    void stopwatch_overflow(uint16_t seconds) {
        // this function is called when the timer overflows, which happens 25
        // seconds after movement stops. if we're in force mode, increase the
        // usage in eeprom otherwise do nothing.
        if (motor_state == ControlState::FORCE_CONTROLLING ||
            motor_state == ControlState::FORCE_CONTROLLING_HOME) {
            status_queue_client.send_brushed_move_status_reporter_queue(
                usage_messages::IncreaseForceTimeUsage{
                    .key = hardware.get_usage_eeprom_config()
                               .get_force_application_time_key(),
                    .seconds = seconds});
        }
    }

    void update_and_start_move() {
        if (queue.try_read_isr(&buffered_move)) {
            if (motor_state == ControlState::FORCE_CONTROLLING ||
                motor_state == ControlState::FORCE_CONTROLLING_HOME) {
                // if we've been applying force before the new move is called
                // add that force application time to the usage storage
                status_queue_client.send_brushed_move_status_reporter_queue(
                    usage_messages::IncreaseForceTimeUsage{
                        .key = hardware.get_usage_eeprom_config()
                                   .get_force_application_time_key(),
                        .seconds = uint16_t(
                            hardware.get_stopwatch_pulses(true) / 2600)});
            }
            motor_state = ControlState::ACTIVE;
            buffered_move.start_encoder_position =
                hardware.get_encoder_pulses();
        }
        if (buffered_move.duty_cycle != 0U) {
            driver_hardware.update_pwm_settings(buffered_move.duty_cycle);
        }
        // clear the old states
        hardware.reset_control();
        hardware.set_stay_enabled(false);
        timeout_ticks = 0;
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
            case MoveStopCondition::sync_line:
            case MoveStopCondition::ignore_stalls:
            case MoveStopCondition::limit_switch_backoff:
                // this is an unused move stop condition for the brushed motor
                // just return with no condition
                // TODO creat can bus error messages and send that instead
                finish_current_move(AckMessageId::complete_without_condition);
                break;
        }
    }

    void homing_stopped() {
        // since the buffered move start position isn't reliable here,
        // and then end position will always be 0, we set the buffered move
        // start position to the difference between the encoder pulse count at
        // the beginning and end this way the usage tracker will know how far
        // the motor moved.
        buffered_move.start_encoder_position =
            buffered_move.start_encoder_position -
            hardware.get_encoder_pulses();
        hardware.reset_encoder_pulses();
        finish_current_move(AckMessageId::stopped_by_condition);
    }

    auto limit_switch_triggered() -> bool {
        return hardware.check_limit_switch();
    }

    auto estop_triggered() -> bool { return hardware.check_estop_in(); }

    void cancel_and_clear_moves(
        can::ids::ErrorCode err_code = can::ids::ErrorCode::hardware,
        can::ids::ErrorSeverity severity =
            can::ids::ErrorSeverity::unrecoverable) {
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
        // likewise if the estop is hit and the motor is gripping something
        if (err_code == can::ids::ErrorCode::labware_dropped ||
            (motor_state == ControlState::FORCE_CONTROLLING &&
             err_code == can::ids::ErrorCode::estop_detected)) {
            hardware.set_stay_enabled(true);
        }

        status_queue_client.send_brushed_move_status_reporter_queue(
            can::messages::ErrorMessage{.message_index = message_index,
                                        .severity = severity,
                                        .error_code = err_code});
        status_queue_client.send_brushed_move_status_reporter_queue(
            usage_messages::IncreaseErrorCount{
                .key =
                    hardware.get_usage_eeprom_config().get_error_count_key()});
        clear_queue_until_empty = true;
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
                   MoveStopCondition::sync_line) {
            hold_encoder_position = hardware.get_encoder_pulses();
            motor_state =
                buffered_move.stop_condition == MoveStopCondition::limit_switch
                    ? ControlState::FORCE_CONTROLLING_HOME
                    : ControlState::FORCE_CONTROLLING;
            // If we're in the gripping state, then we want to signal the
            // hardware not to disable the motor on a
            // brushed_motor_controller::stop which would drop the labware.
            if (motor_state == ControlState::FORCE_CONTROLLING) {
                hardware.set_stay_enabled(true);
            }
        }

        if (buffered_move.group_id != NO_GROUP) {
            auto ack = buffered_move.build_ack(hardware.get_encoder_pulses(), 0,
                                               ack_msg_id);

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

    /**
     * @brief Pop the next message out of the motion queue and discard it.
     *
     * @return true if the queue still has another message, false if this
     * was the last message in the queue.
     */
    auto pop_and_discard_move() -> bool {
        auto scratch = BrushedMove{};
        std::ignore = queue.try_read_isr(&scratch);
        return has_messages();
    }

    std::atomic<bool> is_idle = true;
    uint32_t tick = 0;
    uint32_t timeout_ticks = 0;
    ControlState motor_state = ControlState::IDLE;

  private:
    GenericQueue& queue;
    StatusClient& status_queue_client;
    motor_hardware::BrushedMotorHardwareIface& hardware;
    brushed_motor_driver::BrushedMotorDriverIface& driver_hardware;
    error_tolerance_config::BrushedMotorErrorTolerance& error_conf;
    BrushedMove buffered_move = BrushedMove{};
    int32_t hold_encoder_position = 0;
    uint32_t current_control_pwm = 0;
    bool clear_queue_until_empty = false;
};
}  // namespace brushed_motor_handler
