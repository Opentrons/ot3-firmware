#pragma once
#include <variant>

#include "common/core/message_queue.hpp"
#include "pipettes/core/pipette_messages.hpp"

namespace motor_handler {

using Message = pipette_messages::Move;

template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<Message>, Message>
class MotorInterruptHandler {
  public:
    using GenericQueue = QueueImpl<Message>;

    MotorInterruptHandler() {}

    void set_message_queue(GenericQueue* g_queue) { queue = g_queue; }

    bool has_messages() { return queue->has_message_isr(); }

    bool can_step() { return (has_move && inc < buffered_move.steps); }

    void update_move() {
        finish_current_move();
        get_move(&buffered_move); }

    void finish_current_move() {
        has_move = false;
        buffered_move = Message{};
        inc = 0x0;
    }

    void reset() {
        queue->reset();
        inc = 0x0;
        has_move = false;
    }

    void increment_counter() { inc++; }

  private:
    uint32_t inc = 0x0;
    GenericQueue* queue = nullptr;
    bool has_move = false;
    Message buffered_move = Message{};

    void get_move(Message* msg) { has_move = queue->try_read_isr(msg); }
};
}  // namespace motor_handler
