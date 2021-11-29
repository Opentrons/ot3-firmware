#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

using namespace motor_handler;


void MotorInterruptHandler::run_interrupt() {
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

void MotorInterruptHandler::start() {
    hardware.start_timer_interrupt();
}

void MotorInterruptHandler::stop() {
    hardware.stop_timer_interrupt();
}

    [[nodiscard]] auto MotorInterruptHandler::has_messages() const -> bool {
        return queue.has_message_isr();
    }

    [[nodiscard]] auto MotorInterruptHandler::can_step() const -> bool {
        /*
         * A motor should only try to take a step when the current position
         * does not equal the target position.
         */
        return tick_count < buffered_move.duration;
    }

    [[nodiscard]] auto MotorInterruptHandler::tick() -> bool {
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

    void MotorInterruptHandler::update_move() {
        has_active_move = queue.try_read_isr(&buffered_move);
        // (TODO: lc) We should check the direction (and set respectively)
        // the direction pin for the motor once a move is being pulled off the
        // queue stack. We'll probably want to think about moving the hardware
        // pin configurations out of motion controller.
    }

    [[nodiscard]] auto MotorInterruptHandler::set_direction_pin() const -> bool {
        return (buffered_move.velocity > 0);
    }

    void MotorInterruptHandler::finish_current_move() {
        has_active_move = false;
        tick_count = 0x0;
        if (buffered_move.group_id != NO_GROUP) {
            auto ack =
                Ack{.group_id = buffered_move.group_id,
                    .seq_id = buffered_move.seq_id,
                    .current_position = static_cast<uint32_t>(
                        position_tracker),  // TODO (AA 2021-11-10): convert
                                            // this value to mm instead of steps
                    .ack_id = AckMessageId::complete};
            static_cast<void>(completed_queue.try_write_isr(ack));
        }
        set_buffered_move(Move{});
    }

    [[nodiscard]] auto MotorInterruptHandler::pulse() -> bool {
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
        }
        if (has_active_move && can_step() && tick()) {
            return true;
        }
        if (has_active_move && !can_step()) {
            finish_current_move();
            if (has_messages()) {
                update_move();
                if (can_step() && tick()) {
                    return true;
                }
            }
            return false;
        }
        return false;
    }

    void MotorInterruptHandler::reset() {
        /*
         * Reset the position and all queued moves to the motor interrupt
         * handler.
         */
        queue.reset();
        position_tracker = 0x0;
        tick_count = 0x0;
        has_active_move = false;
    }



    [[nodiscard]] auto MotorInterruptHandler::get_current_position() const -> q31_31 {
        return position_tracker;
    }

    void MotorInterruptHandler::set_current_position(q31_31 pos_tracker) {
        position_tracker = pos_tracker;
    }

    [[nodiscard]] auto MotorInterruptHandler::get_buffered_move() const -> Move {
        return buffered_move;
    }

    void MotorInterruptHandler::set_buffered_move(Move new_move) { buffered_move = new_move; }
