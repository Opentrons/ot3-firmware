#include <stdio.h>

#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/motor_interrupt_handler.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"

static constexpr auto ticks_per_sec = 170000;

auto sec_to_ticks(float seconds) -> ticks { return seconds * ticks_per_sec; }
auto mm_per_sec_to_mm_per_tick(float mm) -> steps_per_tick {
    return static_cast<steps_per_tick>(mm * ticks_per_sec);
}

int main() {
    auto q_new_moves = test_mocks::MockMessageQueue<motor_messages::Move>{};
    auto q_ack = test_mocks::MockMessageQueue<motor_messages::Ack>{};
    auto hardware = test_mocks::MockMotorHardware();

    auto handler =
        motor_handler::MotorInterruptHandler{q_new_moves, q_ack, hardware};

    auto m1 = motor_messages::Move{.duration = sec_to_ticks(.1),
                                   .velocity = 1 << 20,
                                   .acceleration = 0,
                                   .group_id = 0,
                                   .seq_id = 0};
    auto m2 = motor_messages::Move{.duration = sec_to_ticks(.1),
                                   .velocity = -(1 << 19),
                                   .acceleration = 0,
                                   .group_id = 0,
                                   .seq_id = 1};
    q_new_moves.try_write(m2, 0);
    q_new_moves.try_write(m1, 0);

    do {
        auto pulse = handler.pulse();

        printf("%d,%ld,%d\n", pulse, handler.get_current_position(),
               handler.set_direction_pin());
    } while (handler.has_active_move);

    while (q_ack.has_message()) {
        auto ack = motor_messages::Ack{};
        q_ack.try_read(&ack, 0);
        printf("Move done - group-%d seq-%d\n", ack.group_id, ack.seq_id);
    }
}
