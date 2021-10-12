#pragma once
#include <variant>

#include "common/core/message_queue.hpp"
#include "motor_messages.hpp"

using namespace motor_messages;

namespace motor_handler {

/*
 *
 * A motor motion handler class.
 *
 * Generic Queue: A FreeRTOS queue which must match the interface defined
 * in `message_queue.hpp`.
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

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<Move>, Move>
class MotorInterruptHandler {
  public:
    using GenericQueue = QueueImpl<Move>;
    bool has_active_move = false;

    MotorInterruptHandler() {}
    MotorInterruptHandler& operator=(MotorInterruptHandler&) = delete;
    MotorInterruptHandler&& operator=(MotorInterruptHandler&&) = delete;
    MotorInterruptHandler(MotorInterruptHandler&) = delete;
    MotorInterruptHandler(MotorInterruptHandler&&) = delete;

    void set_message_queue(GenericQueue* g_queue) { queue = g_queue; }

    bool has_messages() { return queue->has_message_isr(); }

    bool can_step() {
        /*
         * A motor should only try to take a step when the current position
         * does not equal the target position.
         */
        return (position_tracker != buffered_move.target_position);
    }

    bool tick() {
        /*
         * A function that increments the position tracker of a given motor.
         * The position tracker is the absolute position of a motor which can
         * only ever be positive (inclusive of zero).
         */
        q31_31 old_position = position_tracker;
        buffered_move.velocity += buffered_move.acceleration;
        position_tracker += buffered_move.velocity;
        if (overflow(old_position, position_tracker) == true) {
            buffered_move.target_position = old_position;
            position_tracker = old_position;
            return false;
        }
        return bool((old_position ^ position_tracker) & tick_flag);
    }

    void update_move() {
        has_active_move = queue->try_read_isr(&buffered_move);
        // (TODO: lc) We should check the direction (and set respectively)
        // the direction pin for the motor once a move is being pulled off the
        // queue stack. We'll probably want to think about moving the hardware
        // pin configurations out of motion controller.
    }

    void finish_current_move() {
        has_active_move = false;
        set_buffered_move(Move{});
    }

    bool pulse() {
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
         * This function is called from a timer interrupt. See `step_motor.cpp`.
         */
        if (!has_active_move && has_messages()) {
            update_move();
            return false;
        } else if (has_active_move && can_step() && tick()) {
            return true;
        } else if (has_active_move && !can_step()) {
            finish_current_move();
            return false;
        }
        return false;
    }

    void reset() {
        /*
         * Reset the position and all queued moves to the motor interrupt
         * handler.
         */
        queue->reset();
        position_tracker = 0x0;
        has_active_move = false;
    }

    bool overflow(q31_31 current, q31_31 future) {
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

    q31_31 get_current_position() { return position_tracker; }

    void set_current_position(q31_31 pos_tracker) {
        position_tracker = pos_tracker;
    }

    Move* get_buffered_move() { return &buffered_move; }

    void set_buffered_move(Move new_move) { buffered_move = new_move; }

  private:
    const q31_31 tick_flag = 0x80000000;
    const uint64_t overflow_flag = 0x8000000000000000;
    q31_31 position_tracker = 0x0;
    GenericQueue* queue = nullptr;
    Move buffered_move = Move{};
};
}  // namespace motor_handler
