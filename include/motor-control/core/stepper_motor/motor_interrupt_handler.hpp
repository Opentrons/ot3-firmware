#pragma once

#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

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
 * has_messages -> checks if there are messages available in the queue.
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
 * has_active_move -> True if there is an active move to check whether
 * the motor can step or not. Otherwise False.
 * GenericQueue -> A FreeRTOS queue of any shape.
 *
 * Note: The position tracker should never be allowed to go below zero.
 */

// (TODO lc): This should probably live in the motor configs.
constexpr const int clk_frequency = 85000000 / (5001 * 2);

template <template <class> class QueueImpl,
          move_status_reporter_task::TaskClient StatusClient>
requires MessageQueue<QueueImpl<Move>, Move>
class MotorInterruptHandler {
  public:
    using GenericQueue = QueueImpl<Move>;

    MotorInterruptHandler() = delete;
    MotorInterruptHandler(
        GenericQueue& incoming_queue, StatusClient& outgoing_queue,
        motor_hardware::StepperMotorHardwareIface& hardware_iface)
        : queue(incoming_queue),
          status_queue_client(outgoing_queue),
          hardware(hardware_iface) {}
    ~MotorInterruptHandler() = default;
    auto operator=(MotorInterruptHandler&) -> MotorInterruptHandler& = delete;
    auto operator=(MotorInterruptHandler&&) -> MotorInterruptHandler&& = delete;
    MotorInterruptHandler(MotorInterruptHandler&) = delete;
    MotorInterruptHandler(MotorInterruptHandler&&) = delete;

    // This is the function that should be called by the interrupt callback glue
    // It will run motion math, handle the immediate move buffer, and set or
    // clear step, direction, and sometimes enable pins
    void run_interrupt() {
        if (set_direction_pin()) {
            hardware.positive_direction();
        } else {
            hardware.negative_direction();
        }
        if (pulse()) {
            hardware.step();
        }
        hardware.unstep();
    }

    // Start or stop the handler; this will also start or stop the timer
    void start() { hardware.start_timer_interrupt(); }
    void stop() { hardware.stop_timer_interrupt(); }

    // condense these
    [[nodiscard]] auto pulse() -> bool {
        /*
         * Function to determine whether a motor step-line should
         * be pulsed or not. It should return true if the step-line is to be
         * pulsed, and false otherwise.
         *
         * An active move is true when there is a move that has been
         * pulled from the queue. False otherwise.
         *
         * Logic:
         * 1. If there is not currently an active move, we should check if there
         * are any available on the queue.
         * 2. If there is an active move, and stepping is possible, then we
         * should increment the step counter and return true.
         * 3. Finally, if there is an active move, but you can no longer step
         * then active move should be set to false.
         *
         * This function is called from a timer interrupt. See
         * `motor_hardware.cpp`.
         */
        if (!has_active_move && has_messages()) {
            update_move();
            return false;
        }
        if (has_active_move) {
            if (buffered_move.stop_condition ==
                    MoveStopCondition::limit_switch &&
                homing_stopped()) {
                return false;
            }
            if (buffered_move.stop_condition == MoveStopCondition::cap_sensor &&
                calibration_stopped()) {
                return false;
            }
            if (can_step() && tick()) {
                return true;
            }
            if (!can_step()) {
                finish_current_move();
                if (has_messages()) {
                    update_move();
                    if (can_step() && tick()) {
                        return true;
                    }
                }
                return false;
            }
        }
        return false;
    }

    auto calibration_stopped() -> bool {
        if (sync_triggered()) {
            // return the position
            finish_current_move(AckMessageId::stopped_by_condition);
            return true;
        }
        return false;
    }

    auto homing_stopped() -> bool {
        if (limit_switch_triggered()) {
            position_tracker = 0;
            finish_current_move(AckMessageId::stopped_by_condition);
            reset_encoder_pulses();
            return true;
        }
        return false;
    }

    auto sync_triggered() -> bool { return hardware.check_sync_in(); }

    auto limit_switch_triggered() -> bool {
        return hardware.check_limit_switch();
    }
    [[nodiscard]] auto tick() -> bool {
        /*
         * A function that increments the position tracker of a given motor.
         * The position tracker is the absolute position of a motor which can
         * only ever be positive (inclusive of zero).
         */
        tick_count++;
        q31_31 old_position = position_tracker;
        buffered_move.velocity += buffered_move.acceleration;
        position_tracker += buffered_move.velocity;
        if (overflow(old_position, position_tracker)) {
            position_tracker = old_position;
            return false;
        }
        // check to see when motor should step using velocity & tick count
        return bool((old_position ^ position_tracker) & tick_flag);
    }

    [[nodiscard]] auto has_messages() const -> bool {
        return queue.has_message_isr();
    }
    [[nodiscard]] auto can_step() const -> bool {
        /*
         * A motor should only try to take a step when the current position
         * does not equal the target position.
         */
        return tick_count < buffered_move.duration;
    }

    void update_move() {
        has_active_move = queue.try_read_isr(&buffered_move);
        if (has_active_move &&
            buffered_move.stop_condition == MoveStopCondition::limit_switch) {
            position_tracker = 0x7FFFFFFFFFFFFFFF;
        }
        // (TODO: lc) We should check the direction (and set respectively)
        // the direction pin for the motor once a move is being pulled off the
        // queue stack. We'll probably want to think about moving the hardware
        // pin configurations out of motion controller.
    }

    auto update_encoder_position() -> uint32_t { return get_encoder_pulses(); }

    [[nodiscard]] auto set_direction_pin() const -> bool {
        return (buffered_move.velocity > 0);
    }

    void finish_current_move(
        AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        has_active_move = false;
        tick_count = 0x0;
        if (buffered_move.group_id != NO_GROUP) {
            auto ack = Ack{
                .group_id = buffered_move.group_id,
                .seq_id = buffered_move.seq_id,
                .current_position_steps =
                    static_cast<uint32_t>(position_tracker >> 31),
                .encoder_position = update_encoder_position(),
                .ack_id = ack_msg_id,
            };
            static_cast<void>(
                status_queue_client.send_move_status_reporter_queue(ack));
        }
        set_buffered_move(Move{});
    }

    auto get_encoder_pulses() {
        if (get_encoder_overflow_status() == true) {
            hardware.clear_encoder_SR();
            if (enc_overflow_future_flag) {
                enc_position_tracker = hardware.get_encoder_pulses();
                enc_overflow_future_flag = false;
            } else {
                enc_position_tracker =
                    UINT16_MAX + hardware.get_encoder_pulses();
                enc_overflow_future_flag = true;
            }
        } else {
            enc_position_tracker = hardware.get_encoder_pulses();
        }
        return enc_position_tracker;
        /* This function fixes the overflow issue for each motor.
        Check whether the UIEF interrupt bit in the Status register gets
        triggered. If it gets triggered we save the previous state of the
        triggered flag and clear the status register interrupt bit.
        */
    }

    void reset_encoder_pulses() { hardware.reset_encoder_pulses(); }

    void clear_encoder_overflow_flag() { hardware.clear_encoder_SR(); }

    auto get_encoder_overflow_status() -> bool {
        return hardware.get_encoder_SR_flag();
    }

    void reset() {
        /*
         * Reset the position and all queued moves to the motor interrupt
         * handler.
         */
        queue.reset();
        position_tracker = 0x0;
        tick_count = 0x0;
        has_active_move = false;
    }

    [[nodiscard]] static auto overflow(q31_31 current, q31_31 future) -> bool {
        /*
         * Check whether the position has overflowed. Return true if this has
         * happened, and false otherwise.
         *
         * (TODO lc): If an overflow has happened, we should probably notify the
         * server somehow that the position needed to be capped.
         * Note: could not get the built-ins to work as expected.
         */
        return bool((current ^ future) & overflow_flag);
    }

    // test interface
    [[nodiscard]] auto get_current_position() const -> q31_31 {
        return position_tracker;
    }
    void set_current_position(q31_31 pos_tracker) {
        position_tracker = pos_tracker;
    }
    bool has_active_move = false;
    [[nodiscard]] auto get_buffered_move() const -> Move {
        return buffered_move;
    }
    void set_buffered_move(Move new_move) { buffered_move = new_move; }

  private:
    uint64_t tick_count = 0x0;
    static constexpr const q31_31 tick_flag = 0x80000000;
    static constexpr const uint64_t overflow_flag = 0x8000000000000000;
    q31_31 position_tracker = 0x0;  // in steps
    bool enc_overflow_future_flag = false;
    uint32_t enc_position_tracker = 0x0;
    GenericQueue& queue;
    StatusClient& status_queue_client;
    motor_hardware::StepperMotorHardwareIface& hardware;
    Move buffered_move = Move{};
};
}  // namespace motor_handler
