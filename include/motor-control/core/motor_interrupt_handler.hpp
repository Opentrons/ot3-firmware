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
 * update_move -> public function which calls get move and resets the
 * counter/buffered move.
 * finish_current_move -> reset the counter and the buffered move.
 * reset -> reset the queue and any other private global variables.
 * increment_counter -> increment the counter
 *
 * Private:
 * get_move -> read from the queue to get the next available move message.
 * buffered_move -> The move message with all relevant info to complete a move.
 *
 * Attributes:
 * has_active_move -> True if there is an active move to check whether
 * the motor can step or not. Otherwise False.
 * GenericQueue -> A FreeRTOS queue of any shape.
 */

constexpr const int clk_frequency = 85000000 / (5001 * 2);

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<Move>, Move>
class MotorInterruptHandler {
  public:
    using GenericQueue = QueueImpl<Move>;

    MotorInterruptHandler() {}

    void set_message_queue(GenericQueue* g_queue) { queue = g_queue; }

    bool has_messages() { return queue->has_message_isr(); }

    bool can_step() {
        return (has_active_move && step_count <= buffered_move.target_position);
    }

    bool tick() {
        sq32_31 old_step_count = step_count;
        step_count += steps_per_tick;
        return bool((old_step_count ^ step_count) & 0x100000000);
    }

    void update_move() {
        finish_current_move();
        get_move(&buffered_move);
    }

    void finish_current_move() {
        has_active_move = false;
        buffered_move = Move{};
    }

    void reset() {
        queue->reset();
        step_count = 0x0;
        has_active_move = false;
    }

    sq32_31 get_current_position() { return step_count; }

    Move get_buffered_move() { return buffered_move; }

  private:
    sq0_31 steps_per_tick = 0x40000000;  // 0.5 steps per tick
    sq32_31 step_count = 0x0;
    uint32_t steps_per_sec = clk_frequency * steps_per_tick;
    GenericQueue* queue = nullptr;
    bool has_active_move = false;
    Move buffered_move = Move{};

    void get_move(Move* msg) { has_active_move = queue->try_read_isr(msg); }
};
}  // namespace motor_handler
