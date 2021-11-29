#pragma once
#include <variant>

#include "common/core/freertos_message_queue.hpp"
#include "motor_messages.hpp"

namespace motor_hardware {
class MotorHardwareIface;
};

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

class MotorInterruptHandler {
  public:
    using GenericQueue = freertos_message_queue::FreeRTOSMessageQueue<Move>;
    using CompletedQueue = freertos_message_queue::FreeRTOSMessageQueue<Ack>;

    MotorInterruptHandler() = delete;
    MotorInterruptHandler(GenericQueue& incoming_queue,
                          CompletedQueue& outgoing_queue,
                          motor_hardware::MotorHardwareIface& hardware_iface)
        : queue(incoming_queue),
          completed_queue(outgoing_queue),
          hardware(hardware_iface) {}
    ~MotorInterruptHandler() = default;
    auto operator=(MotorInterruptHandler&) -> MotorInterruptHandler& = delete;
    auto operator=(MotorInterruptHandler&&) -> MotorInterruptHandler&& = delete;
    MotorInterruptHandler(MotorInterruptHandler&) = delete;
    MotorInterruptHandler(MotorInterruptHandler&&) = delete;

    // This is the function that should be called by the interrupt callback glue
    // It will run motion math, handle the immediate move buffer, and set or
    // clear step, direction, and sometimes enable pins
    void run_interrupt();

    // Start or stop the handler; this will also start or stop the timer
    void start();
    void stop();

    // condense these
    [[nodiscard]] auto pulse() -> bool;
    [[nodiscard]] auto tick() -> bool;

    [[nodiscard]] auto has_messages() const -> bool;
    [[nodiscard]] auto can_step() const -> bool;

    void update_move();

    [[nodiscard]] auto set_direction_pin() const -> bool;

    void finish_current_move();

    void reset();

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
    [[nodiscard]] auto get_current_position() const -> q31_31;
    void set_current_position(q31_31 pos_tracker);
    bool has_active_move = false;
    [[nodiscard]] auto get_buffered_move() const -> Move;
    void set_buffered_move(Move new_move);

  private:
    uint64_t tick_count = 0x0;
    static constexpr const q31_31 tick_flag = 0x80000000;
    static constexpr const uint64_t overflow_flag = 0x8000000000000000;
    q31_31 position_tracker = 0x0;  // in steps
    GenericQueue& queue;
    CompletedQueue& completed_queue;
    motor_hardware::MotorHardwareIface& hardware;
    Move buffered_move = Move{};
};
}  // namespace motor_handler
