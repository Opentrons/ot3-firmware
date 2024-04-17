#pragma once

#include <atomic>

#include "can/core/ids.hpp"
#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/stall_check.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "motor-control/core/tasks/tmc_motor_driver_common.hpp"

namespace motor_handler {

using namespace motor_messages;
/*
 *
 * A motor motion handler class.
 *
 *
 * Public:
 * set_message_queue -> set the queue to be used by the global motor handler in
 * the `step_motor.cpp` file.
 * has_move_messages -> checks if there are messages available in the queue.
 * can_step -> determines whether a motor can step or not. Currently, it only
 * checks to see whether the incrementer is less than the steps to move.
 * update_move -> public function which calls get move and resets the buffered
 * move. finish_current_move -> reset the counter and the buffered move. reset
 * -> reset the queue and any other private global variables. increment_counter
 * -> increment the counter
 *
 * Private:
 * get_move -> read from the queue to get the next available move message.
 * buffered_move -> The move message with all relevant info to complete a move.
 *
 * Attributes:
 * _has_active_move -> True if there is an active move to check whether
 * the motor can step or not. Otherwise False.
 * MoveQueue -> A FreeRTOS queue of any shape.
 *
 * Note: The position tracker should never be allowed to go below zero.
 */

// take macros up a level to this?
//see tabs
struct Empty {};

template <class SensorClient>
struct SensorAccessHelper {
    inline static void send_to_pressure_sensor_queue(SensorClient& sensor_client, can::messages::BindSensorOutputRequest& m) {
        sensor_client.send_to_pressure_sensor_queue(m); // handle return type/data. use send_to_isr like previously
    }
    inline static void send_to_capacitive_sensor_queue(SensorClient& sensor_client, can::messages::BindSensorOutputRequest& m) {
        sensor_client.send_to_capacitive_sensor_queue(m);
    }
}

template<> struct SensorAccessHelper<Empty> {
    inline static void send_to_pressure_sensor_queue(Empty& sensor_client, can::messages::BindSensorOutputRequest& m) {}
    inline static void send_to_capacitive_sensor_queue(Empty& sensor_client, can::messages::BindSensorOutputRequest& m) {}
}

template <template <class> class QueueImpl, class StatusClient,
          class DriverClient, typename MotorMoveMessage, typename MotorHardware,
          class SensorClient = Empty>
requires MessageQueue<QueueImpl<MotorMoveMessage>, MotorMoveMessage> &&
    std::is_base_of_v<motor_hardware::MotorHardwareIface, MotorHardware>
class MotorInterruptHandler {
  public:
    using MoveQueue = QueueImpl<MotorMoveMessage>;
    using UpdatePositionQueue =
        QueueImpl<can::messages::UpdateMotorPositionEstimationRequest>;
    MotorInterruptHandler() = delete;
    MotorInterruptHandler(MoveQueue& incoming_move_queue,
                          StatusClient& outgoing_queue,
                          DriverClient& driver_queue,
                          MotorHardware& hardware_iface,
                          stall_check::StallCheck& stall,
                          UpdatePositionQueue& incoming_update_position_queue,
                          SensorClient& sensor_queue_client) // make optional, default nullopt
        : move_queue(incoming_move_queue),
          status_queue_client(outgoing_queue),
          driver_client(driver_queue),
          hardware(hardware_iface),
          stall_checker{stall},
          update_position_queue(incoming_update_position_queue),
          sensor_client(sensor_client) {
        hardware.unstep();
    }
    ~MotorInterruptHandler() = default;
    auto operator=(MotorInterruptHandler&) -> MotorInterruptHandler& = delete;
    auto operator=(MotorInterruptHandler&&) -> MotorInterruptHandler&& = delete;
    MotorInterruptHandler(MotorInterruptHandler&) = delete;
    MotorInterruptHandler(MotorInterruptHandler&&) = delete;

    // This is the function that should be called by the interrupt callback glue
    // It will run motion math, handle the immediate move buffer, and set or
    // clear step, direction, and sometimes enable pins
    void run_normal_interrupt() {
        if (pulse()) {
            hardware.step();
            update_hardware_step_tracker();
            if (stall_checker.step_itr(set_direction_pin())) {
                if (check_for_stall()) {
                    handle_stall_during_movement();
                }
            }
            hardware.unstep();
        }
    }

    auto check_for_stall() -> bool {
        if (stall_detected()) {
            hardware.position_flags.clear_flag(
                MotorPositionStatus::Flags::stepper_position_ok);
            return true;
        }
        return false;
    }

    auto handle_stall_during_movement() -> void {
        if (!_has_active_move or
            hardware.position_flags.check_flag(
                MotorPositionStatus::Flags::stepper_position_ok) or
            buffered_move.check_stop_condition(
                MoveStopCondition::limit_switch) or
            buffered_move.check_stop_condition(
                MoveStopCondition::limit_switch_backoff)) {
            return;
        }

        if (buffered_move.check_stop_condition(MoveStopCondition::stall)) {
            // if expected, finish move and clear queue to prepare for position
            // update
            finish_current_move(AckMessageId::stopped_by_condition);
            clear_queue_until_empty = true;
        } else if (buffered_move.check_stop_condition(
                       MoveStopCondition::ignore_stalls)) {
            if (stall_handled) {
                return;
            }
            // send a warning
            status_queue_client.send_move_status_reporter_queue(
                can::messages::ErrorMessage{
                    .message_index = buffered_move.message_index,
                    .severity = can::ids::ErrorSeverity::warning,
                    .error_code = can::ids::ErrorCode::collision_detected});
            status_queue_client.send_move_status_reporter_queue(
                usage_messages::IncreaseErrorCount{
                    .key = hardware.get_usage_eeprom_config()
                               .get_error_count_key()});
        } else {
            cancel_and_clear_moves(can::ids::ErrorCode::collision_detected);
        }
        stall_handled = true;
    }

    auto stall_detected() -> bool {
        return !stall_checker.check_stall_itr(hardware.get_encoder_pulses());
    }

    auto estop_update() -> bool {
        /**
         * Run tasks that need to occur in the steady state where the estop
         * remains triggered.
         *
         * Returns the current state of the estop line.
         * */
        auto estop_status = estop_triggered();
        if (!estop_status) {
            // estop is now not triggered, say we're good
            status_queue_client.send_move_status_reporter_queue(
                can::messages::ErrorMessage{
                    .message_index = 0,
                    .severity = can::ids::ErrorSeverity::warning,
                    .error_code = can::ids::ErrorCode::estop_released});
        } else {
            if (has_move_messages()) {
                // If we're currently in the estop and we have messages, then
                // somebody sent us new moves while we were in the estop.

                // Nothing has set clear_queue_until_empty. That flag is set
                // from false to true by the code below; by
                // cancel_and_clear_moves; and nowhere else. That means that
                // no move from the group that is in the queue was currently
                // being executed when estop asserted (or it would have been
                // cancelled by cancel_and_clear_moves) and no move from the
                // group that is in the queue has been responded to with an
                // error (or the code below would have run and set the
                // flag). Therefore, we need to send an error, and set the
                // flag to consume the rest of the group.
                auto scratch = MotorMoveMessage{};
                std::ignore = move_queue.try_read_isr(&scratch);
                status_queue_client.send_move_status_reporter_queue(
                    can::messages::ErrorMessage{
                        .message_index = scratch.message_index,
                        .severity = can::ids::ErrorSeverity::unrecoverable,
                        .error_code = can::ids::ErrorCode::estop_detected});
                status_queue_client.send_move_status_reporter_queue(
                    usage_messages::IncreaseErrorCount{
                        .key = hardware.get_usage_eeprom_config()
                                   .get_error_count_key()});
                clear_queue_until_empty = move_queue.has_message_isr();
            }
        }
        return estop_status;
    }

    void run_interrupt() {
        // handle various error states
        motor_hardware::CancelRequest cancel_request =
            hardware.get_cancel_request();
        std::ignore = hardware.get_encoder_pulses();
        if (clear_queue_until_empty) {
            // If we were executing a move when estop asserted, and
            // what's in the queue is the remaining enqueued moves from
            // that group, then we will have called
            // cancel_and_clear_moves and clear_queue_until_empty will
            // be true. That means we should pop out the queue.
            // clear_queue_until_empty will also be true if we were in
            // the steady-state estop asserted; got new messages; and
            // the other branch of this if asserted. In either case, an
            // error message has been sent, so we need to just keep
            // clearing the queue.
            clear_queue_until_empty = pop_and_discard_move();
        } else if (in_estop) {
            handle_update_position_queue();
            in_estop = estop_update();
        } else if (estop_triggered()) {
            cancel_and_clear_moves(can::ids::ErrorCode::estop_detected);
            in_estop = true;
        } else if (static_cast<can::ids::ErrorCode>(cancel_request.code) !=
                   can::ids::ErrorCode::ok) {
            cancel_and_clear_moves(
                static_cast<can::ids::ErrorCode>(cancel_request.code),
                static_cast<can::ids::ErrorSeverity>(cancel_request.severity));
        } else {
            // Normal Move logic
            run_normal_interrupt();
        }
    }

    // Start or stop the handler; this will also start or stop the timer
    void start() { hardware.start_timer_interrupt(); }
    void stop() { hardware.stop_timer_interrupt(); }

    [[nodiscard]] auto stop_condition_met() {
        if (buffered_move.check_stop_condition(
                MoveStopCondition::limit_switch) &&
            homing_stopped()) {
            return true;
        }
        if (buffered_move.check_stop_condition(
                MoveStopCondition::limit_switch_backoff) &&
            backed_off()) {
            return true;
        }
        if (buffered_move.check_stop_condition(MoveStopCondition::sync_line) &&
            sync_triggered()) {
            return true;
        }
        return false;
    }

    // condense these
    [[nodiscard]] auto pulse() -> bool {
        /*
         * Function to determine whether a motor step-line should
         * be pulsed or not. It should return true if the step-line is to be
         * pulsed, and false otherwise.
         *
         * An active move is true when there is a move that has been
         * pulled from the move_queue. False otherwise.
         *
         * Logic:
         * 1. If there is not currently an active move, we should check if there
         * are any available on the move_queue.
         * 2. If there is an active move, and stepping is possible, then we
         * should increment the step counter and return true.
         * 3. Finally, if there is an active move, but you can no longer step
         * then active move should be set to false.
         *
         * This function is called from a timer interrupt. See
         * `motor_hardware.cpp`.
         */
        if (!_has_active_move && has_move_messages()) {
            update_move();
            handle_update_position_queue_error();
            return false;
        }
        if (_has_active_move) {
            handle_update_position_queue_error();
            if (stop_condition_met()) {
                return false;
            }
            if (can_step() && tick()) {
                return true;
            }
            if (!can_step()) {
                finish_current_move();
                if (has_move_messages()) {
                    update_move();
                    if (can_step() && tick()) {
                        return true;
                    }
                }
                return false;
            }
        } else {
            if (update_position_queue.has_message_isr()) {
                handle_update_position_queue();
            }
        }
        return false;
    }

    auto homing_stopped() -> bool {
        if (limit_switch_triggered()) {
            position_tracker = 0;
            hardware.reset_step_tracker();
            finish_current_move(AckMessageId::stopped_by_condition);
            return true;
        }
        return false;
    }

    auto backed_off() -> bool {
        if (!limit_switch_triggered()) {
            position_tracker = 0;
            // since the buffered move start position isn't reliable here,
            // and then end position will always be 0, we set the buffered move
            // start position to the difference between the encoder pulse count
            // at the beginning and end this way the usage tracker will know how
            // far the motor moved.
            buffered_move.start_encoder_position =
                buffered_move.start_encoder_position -
                hardware.get_encoder_pulses();
            hardware.reset_step_tracker();
            hardware.reset_encoder_pulses();

            stall_checker.reset_itr_counts(0);
            hardware.position_flags.set_flag(
                can::ids::MotorPositionFlags::stepper_position_ok);
            if (stall_checker.has_encoder()) {
                hardware.position_flags.set_flag(
                    can::ids::MotorPositionFlags::encoder_position_ok);
            }
            finish_current_move(AckMessageId::stopped_by_condition);
            return true;
        }
        return false;
    }

    auto sync_triggered() -> bool {
        if (hardware.check_sync_in()) {
            finish_current_move(AckMessageId::stopped_by_condition);
            return true;
        }
        return false;
    }

    auto limit_switch_triggered() -> bool {
        return hardware.check_limit_switch();
    }

    auto estop_triggered() -> bool { return hardware.check_estop_in(); }

    [[nodiscard]] auto tick() -> bool {
        /*
         * A function that increments the position tracker of a given motor.
         * The position tracker is the absolute position of a motor which can
         * only ever be positive (inclusive of zero).
         *
         * The caller must update the step position to the hardware iface
         * if necessary.
         */
        tick_count++;
        buffered_move.velocity += buffered_move.acceleration;
        auto old_position = position_tracker;
        position_tracker += buffered_move.velocity;
        if (overflow(old_position, position_tracker)) {
            position_tracker = old_position;
            // Don't need to sync the hardware step counter
            return false;
        }
        // check to see when motor should step using velocity & tick count
        return bool((old_position ^ position_tracker) & tick_flag);
    }

    [[nodiscard]] auto has_move_messages() const -> bool {
        return move_queue.has_message_isr();
    }
    [[nodiscard]] auto can_step() const -> bool {
        /*
         * A motor should only try to take a step when the current position
         * does not equal the target position.
         */
        return tick_count < buffered_move.duration;
    }

    void update_move() {
        _has_active_move = move_queue.try_read_isr(&buffered_move);
        if (_has_active_move) {
            hardware.enable_encoder();
            buffered_move.start_encoder_position =
                hardware.get_encoder_pulses();
#ifdef USE_SENSOR_MOVE
            if (buffered_move.sensor_id != can::ids::SensorId::UNUSED) {
                if (buffered_move.sensor_type == can::ids::SensorType::pressure) {
                    auto msg = can::messages::BindSensorOutputRequest{
                        .message_index = buffered_move.message_index,
                        .sensor = can::ids::SensorType::pressure,
                        .sensor_id = buffered_move.sensor_id,
                        .binding = static_cast<uint8_t>(0x3)  // sync and report
                    };
                    send_to_pressure_sensor_queue(msg);
                } else if (buffered_move.sensor_type == can::ids::SensorType::capacitive) {
                    auto msg = can::messages::BindSensorOutputRequest{
                        .message_index = buffered_move.message_index,
                        .sensor = can::ids::SensorType::capacitive,
                        .sensor_id = buffered_move.sensor_id,
                        .binding = static_cast<uint8_t>(0x3)  // sync and report
                    };
                    send_to_capacitive_sensor_queue(msg);
                }
            }
#endif
        }
        if (set_direction_pin()) {
            hardware.positive_direction();
        } else {
            hardware.negative_direction();
        }
        if (_has_active_move && buffered_move.check_stop_condition(
                                    MoveStopCondition::limit_switch)) {
            position_tracker = 0x7FFFFFFFFFFFFFFF;
            update_hardware_step_tracker();
            hardware.position_flags.clear_flag(
                can::ids::MotorPositionFlags::stepper_position_ok);
            hardware.position_flags.clear_flag(
                can::ids::MotorPositionFlags::encoder_position_ok);
        }
    }

    /**
     * @brief Pop the next message out of the motion queue and discard it.
     *
     * @return true if the queue still has another message, false if this
     * was the last message in the queue.
     */
    auto pop_and_discard_move() -> bool {
        auto scratch = MotorMoveMessage{};
        std::ignore = move_queue.try_read_isr(&scratch);
        return has_move_messages();
    }

    [[nodiscard]] auto set_direction_pin() const -> bool {
        return (buffered_move.velocity > 0);
    }
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
        status_queue_client.send_move_status_reporter_queue(
            can::messages::ErrorMessage{.message_index = message_index,
                                        .severity = severity,
                                        .error_code = err_code});
        if (err_code == can::ids::ErrorCode::motor_driver_error_detected) {
            driver_client.send_motor_driver_queue_isr(
                can::messages::ReadMotorDriverErrorStatusRequest{
                    .message_index = message_index});
        }
        if (err_code == can::ids::ErrorCode::collision_detected) {
            build_and_send_ack(AckMessageId::position_error);
        }
        status_queue_client.send_move_status_reporter_queue(
            usage_messages::IncreaseErrorCount{
                .key =
                    hardware.get_usage_eeprom_config().get_error_count_key()});

        // We have to make sure that
        // other steps in the queue DO NOT execute. With this flag we
        // will clear out the interrupt's queue.
        clear_queue_until_empty = true;

        // the queue will get reset during the stop message processing
        // we can't clear here from an interrupt context
        _has_active_move = false;
        tick_count = 0x0;
    }

    void build_and_send_ack(AckMessageId ack_msg_id) {
        if (buffered_move.group_id != NO_GROUP) {
            auto ack = buffered_move.build_ack(
                hardware.get_step_tracker(), hardware.get_encoder_pulses(),
                hardware.position_flags.get_flags(), ack_msg_id);
            static_cast<void>(
                status_queue_client.send_move_status_reporter_queue(ack));
        }
    }

    void finish_current_move(
        AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        _has_active_move = false;
        tick_count = 0x0;
        stall_handled = false;
        build_and_send_ack(ack_msg_id);
#ifdef USE_SENSOR_MOVE
        if (buffered_move.sensor_id != can::ids::SensorId::UNUSED) {
            if (buffered_move.sensor_type == can::ids::SensorType::pressure) {
                auto stop_msg = can::messages::BindSensorOutputRequest{
                    .message_index = buffered_move.message_index,
                    .sensor = can::ids::SensorType::pressure,
                    .sensor_id = buffered_move.sensor_id,
                    .binding =
                        static_cast<uint8_t>(can::ids::SensorOutputBinding::sync)};
                send_to_pressure_sensor_queue(stop_msg);
            } else if (buffered_move.sensor_type == can::ids::SensorType::pressure) {
                auto stop_msg = can::messages::BindSensorOutputRequest{
                    .message_index = buffered_move.message_index,
                    .sensor = can::ids::SensorType::capacitive,
                    .sensor_id = buffered_move.sensor_id,
                    .binding =
                        static_cast<uint8_t>(can::ids::SensorOutputBinding::sync)};
                send_to_capacitive_sensor_queue(stop_msg);
            }
        }
#endif
        set_buffered_move(MotorMoveMessage{});
        // update the stall check ideal encoder counts based on
        // last known location
        if (!has_move_messages()) {
            stall_checker.reset_itr_counts(hardware.get_step_tracker());
        }
    }

    void reset() {
        /*
         * Reset the position and all queued moves to the motor interrupt
         * handler.
         */
        move_queue.reset();
        update_position_queue.reset();
        position_tracker = 0;
        update_hardware_step_tracker();
        tick_count = 0x0;
        _has_active_move = false;
        hardware.reset_encoder_pulses();
        stall_checker.reset_itr_counts(0);
        stall_handled = false;
    }

    [[nodiscard]] static auto overflow(q31_31 current, q31_31 future) -> bool {
        /*
         * Check whether the position has overflowed. Return true if this has
         * happened, and false otherwise.
         *
         * (TODO lc): If an overflow has happened, we should
         * probably notify the server somehow that the position needed to be
         * capped. Note: could not get the built-ins to work as expected.
         */
        return bool((current ^ future) & overflow_flag);
    }

    // test interface
    [[nodiscard]] auto get_current_position() const -> q31_31 {
        return position_tracker;
    }
    void set_current_position(q31_31 pos_tracker) {
        position_tracker = pos_tracker;
        update_hardware_step_tracker();
    }

    [[nodiscard]] auto get_buffered_move() const -> MotorMoveMessage {
        return buffered_move;
    }
    void set_buffered_move(MotorMoveMessage new_move) {
        buffered_move = new_move;
    }

    /**
     * @brief While a move is NOT active, this function should be called
     * to check if there is a pending UpdateMotorPositionEstimationRequest
     * message and update the motor position if such a message exists.
     */
    auto handle_update_position_queue() -> void {
        can::messages::UpdateMotorPositionEstimationRequest msg = {};

        if (update_position_queue.try_read_isr(&msg)) {
            auto encoder_pulses = hardware.get_encoder_pulses();
            // Only update position if:
            // 1. We are not moving
            // 2. The encoder position is valid
            // 3. We have an encoder (implicitly checked by 2)
            if (!_has_active_move &&
                hardware.position_flags.check_flag(
                    MotorPositionStatus::Flags::encoder_position_ok)) {
                encoder_pulses = address_negative_encoder();
                auto stepper_tick_estimate =
                    stall_checker.encoder_ticks_to_stepper_ticks(
                        encoder_pulses);
                set_current_position(static_cast<q31_31>(stepper_tick_estimate)
                                     << 31);
                stall_checker.reset_itr_counts(stepper_tick_estimate);
                hardware.position_flags.set_flag(
                    MotorPositionStatus::Flags::stepper_position_ok);
            }
            // We send an ack even if the position wasn't updated
            auto ack = motor_messages::UpdatePositionResponse{
                .message_index = msg.message_index,
                .stepper_position_counts = hardware.get_step_tracker(),
                .encoder_pulses = encoder_pulses,
                .position_flags = hardware.position_flags.get_flags()};
            static_cast<void>(
                status_queue_client.send_move_status_reporter_queue(ack));
        }
    }

    /**
     * @brief While a move is active, this function should be called to
     * check if there is a pending UpdateMotorPositionEstimationRequest and
     * send an error message over CAN if such a message exists.
     *
     */
    auto handle_update_position_queue_error() -> void {
        if (!update_position_queue.has_message_isr()) {
            return;
        }

        can::messages::UpdateMotorPositionEstimationRequest msg = {};
        if (update_position_queue.try_read_isr(&msg)) {
            auto response = can::messages::ErrorMessage{
                .message_index = msg.message_index,
                .severity = can::ids::ErrorSeverity::warning,
                .error_code = can::ids::ErrorCode::motor_busy,
            };

            static_cast<void>(
                status_queue_client.send_move_status_reporter_queue(response));
            status_queue_client.send_move_status_reporter_queue(
                usage_messages::IncreaseErrorCount{
                    .key = hardware.get_usage_eeprom_config()
                               .get_error_count_key()});
        }
    }

    auto address_negative_encoder() -> int32_t {
        auto pulses = hardware.get_encoder_pulses();
        if (pulses < 0) {
            hardware.reset_encoder_pulses();
            return 0;
        }
        return pulses;
    }

    auto has_active_move() -> bool { return _has_active_move.load(); }

    /**
     * @brief Returns whether the motor has an active move *and*
     * the velocity is nonzero.
     */
    auto is_moving() -> bool {
        return has_active_move() && buffered_move.velocity != 0;
    }

  protected:
    void update_hardware_step_tracker() {
        hardware.set_step_tracker(
            static_cast<uint32_t>(position_tracker >> 31));
    }
#ifdef USE_SENSOR_MOVE
    void send_to_pressure_sensor_queue(
        can::messages::BindSensorOutputRequest& m) {
        SensorClientHelper<SensorClient>::send_to_pressure_sensor_queue(sensor_client, m);
    }
    void send_to_capacitive_sensor_queue(
        can::messages::BindSensorOutputRequest& m) {
        SensorClientHelper<SensorClient>::send_to_capacitive_sensor_queue(sensor_client, m);
    }
#endif
    uint64_t tick_count = 0x0;
    static constexpr const q31_31 tick_flag = 0x80000000;
    static constexpr const uint64_t overflow_flag = 0x8000000000000000;
    // Tracks position with sub-microstep accuracy
    q31_31 position_tracker{0};
    MoveQueue& move_queue;
    StatusClient& status_queue_client;
    DriverClient& driver_client;
    MotorHardware& hardware;
    stall_check::StallCheck& stall_checker;
    UpdatePositionQueue& update_position_queue;
    SensorClient& sensor_client;
    MotorMoveMessage buffered_move = MotorMoveMessage{};
    bool clear_queue_until_empty = false;
    bool stall_handled = false;
    bool in_estop = false;
    std::atomic_bool _has_active_move = false;
};
}  // namespace motor_handler
