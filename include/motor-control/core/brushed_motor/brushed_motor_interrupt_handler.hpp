#pragma once

#include <algorithm>
#include <atomic>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/debounce.hpp"
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
            finish_current_move(false, AckMessageId::timeout);
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
                    finish_current_move(true,
                                        AckMessageId::stopped_by_condition);
                }
                break;
            }
            // grip move
            case MoveStopCondition::none:
            case MoveStopCondition::gripper_force:
            case MoveStopCondition::stall:
                if (is_sensing() && is_idle) {
                    finish_current_move(
                        true, AckMessageId::complete_without_condition);
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
        } else if (!error_handled) {
            auto motor_state = hardware.get_motor_state();
            auto pulses = hardware.get_encoder_pulses();
            // has not reported an error yet
            if (motor_state == BrushedMotorState::POSITION_CONTROLLING) {
                int32_t move_delta = pulses - hold_encoder_position;
                controlled_move_to(move_delta);
                // we use a value higher than the acceptable position here to
                // allow the pid loop the opportunity to maintain small
                // movements that occur from motion and vibration
                enc_errored.debounce_update(
                    move_delta > error_conf.unwanted_movement_threshold);
                if (enc_errored.debounce_state()) {
                    // enc value errored for 3 consecutive ticks
                    cancel_and_clear_moves(
                        can::ids::ErrorCode::collision_detected);
                    report_position(pulses);
                    error_handled = true;
                }
            } else if (motor_state != BrushedMotorState::UNHOMED) {
                auto pulses = hardware.get_encoder_pulses();
                enc_errored.debounce_update(
                    !is_idle && pulses >= 0 &&
                    std::abs(pulses - hold_encoder_position) >
                        error_conf.unwanted_movement_threshold);

                if (enc_errored.debounce_state()) {
                    // enc value errored for 3 consecutive ticks
                    // we have likely dropped a labware or had a collision
                    auto err =
                        motor_state == BrushedMotorState::FORCE_CONTROLLING
                            ? can::ids::ErrorCode::labware_dropped
                            : can::ids::ErrorCode::collision_detected;
                    cancel_and_clear_moves(err);
                    report_position(pulses);
                    error_handled = true;
                }
            }
        }
    }

    void run_interrupt() {
        if (clear_queue_until_empty) {
            clear_queue_until_empty = pop_and_discard_move();
        } else if (in_estop) {
            // if we've received a stop request during this time we can clear
            // that flag since there isn't anything running
            std::ignore = hardware.has_cancel_request();
            // return out of error state once the estop is disabled
            if (!estop_triggered()) {
                in_estop = false;
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
            in_estop = true;
            cancel_and_clear_moves(can::ids::ErrorCode::estop_detected);
        } else if (hardware.has_cancel_request()) {
            if (!hardware.get_stay_enabled()) {
                hardware.set_motor_state(BrushedMotorState::UNHOMED);
            }
            cancel_and_clear_moves(can::ids::ErrorCode::stop_requested,
                                   can::ids::ErrorSeverity::warning);
        } else if (tick < error_conf.idle_holdoff_ticks) {
            tick++;
        } else if (_has_active_move) {
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
                 tick < error_conf.idle_holdoff_ticks);
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
        auto motor_state = hardware.get_motor_state();
        if (motor_state == BrushedMotorState::FORCE_CONTROLLING ||
            motor_state == BrushedMotorState::FORCE_CONTROLLING_HOME) {
            status_queue_client.send_brushed_move_status_reporter_queue(
                usage_messages::IncreaseForceTimeUsage{
                    .key = hardware.get_usage_eeprom_config()
                               .get_force_application_time_key(),
                    .seconds = seconds});
        }
    }

    void update_and_start_move() {
        _has_active_move = queue.try_read_isr(&buffered_move);
        if (!_has_active_move) {
            return;
        }
        auto motor_state = hardware.get_motor_state();
        if (motor_state == BrushedMotorState::FORCE_CONTROLLING ||
            motor_state == BrushedMotorState::FORCE_CONTROLLING_HOME) {
            // if we've been applying force before the new move is called
            // add that force application time to the usage storage
            status_queue_client.send_brushed_move_status_reporter_queue(
                usage_messages::IncreaseForceTimeUsage{
                    .key = hardware.get_usage_eeprom_config()
                               .get_force_application_time_key(),
                    .seconds =
                        uint16_t(hardware.get_stopwatch_pulses(true) / 2600)});
        }
        buffered_move.start_encoder_position = hardware.get_encoder_pulses();

        if (buffered_move.duty_cycle != 0U) {
            driver_hardware.update_pwm_settings(buffered_move.duty_cycle);
        }
        // clear the old states
        hardware.reset_control();
        timeout_ticks = 0;
        error_handled = false;
        enc_errored.reset();
        hardware.set_stay_enabled(
            static_cast<bool>(buffered_move.stay_engaged));
        switch (buffered_move.stop_condition) {
            case MoveStopCondition::limit_switch:
                hardware.set_motor_state(
                    BrushedMotorState::FORCE_CONTROLLING_HOME);
                tick = 0;
                hardware.ungrip();
                break;
            case MoveStopCondition::encoder_position:
                hardware.set_motor_state(
                    BrushedMotorState::POSITION_CONTROLLING);
                if (hardware.get_encoder_pulses() ==
                    buffered_move.encoder_position) {
                    LOG("Attempting to add a gripper move to our current "
                        "position");
                    finish_current_move(true,
                                        AckMessageId::stopped_by_condition);
                }
                break;
            case MoveStopCondition::none:
            case MoveStopCondition::gripper_force:
            case MoveStopCondition::stall:
                hardware.set_motor_state(BrushedMotorState::FORCE_CONTROLLING);
                tick = 0;
                hardware.grip();
                break;
            case MoveStopCondition::sync_line:
            case MoveStopCondition::ignore_stalls:
            case MoveStopCondition::limit_switch_backoff:
                // this is an unused move stop condition for the brushed motor
                // just return with no condition
                // TODO creat can bus error messages and send that instead
                finish_current_move(false,
                                    AckMessageId::complete_without_condition);
                break;
        }
    }

    void report_position(int32_t pulses) {
        // this message is used only to report encoder position on the can bus
        // when the move fails, and will not be used nor handled by the host
        uint32_t message_index = 0;
        if (_has_active_move) {
            message_index = buffered_move.message_index;
        }
        status_queue_client.send_brushed_move_status_reporter_queue(
            motor_messages::UpdatePositionResponse{
                .message_index = message_index,
                .stepper_position_counts = 0,
                .encoder_pulses = pulses,
                .position_flags = 0});
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
        finish_current_move(true, AckMessageId::stopped_by_condition);
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
        if (_has_active_move) {
            message_index = buffered_move.message_index;
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
        bool update_hold_position,
        AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        _has_active_move = false;

        // only update hold position when the move is valid
        if (update_hold_position) {
            // use the commanded move position as the hold position if posiiton
            // controling otherwise, we can use the actual jaw encoder position
            hold_encoder_position =
                hardware.get_motor_state() ==
                        BrushedMotorState::POSITION_CONTROLLING
                    ? buffered_move.encoder_position
                    : hardware.get_encoder_pulses();
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

    auto has_active_move() -> bool { return _has_active_move.load(); }

    [[nodiscard]] auto get_idle_holdoff_ticks() const -> uint32_t {
        return error_conf.idle_holdoff_ticks;
    }

    std::atomic<bool> is_idle = true;
    uint32_t tick = 0;
    uint32_t timeout_ticks = 0;
    bool error_handled = false;
    bool in_estop = false;

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
    std::atomic_bool _has_active_move = false;
    debouncer::Debouncer enc_errored = debouncer::Debouncer{};
};
}  // namespace brushed_motor_handler
