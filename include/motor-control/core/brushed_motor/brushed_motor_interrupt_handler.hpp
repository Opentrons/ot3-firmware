#pragma once

#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

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
        motor_hardware::BrushedMotorHardwareIface& hardware_iface)
        : queue(incoming_queue),
          status_queue_client(outgoing_queue),
          hardware(hardware_iface) {}
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

    void run_interrupt() {
        if (!has_active_move && has_messages()) {
            update_move();
        }
        if (has_active_move && limit_switch_triggered()) {
            hardware.stop_pwm();
            if (buffered_move.stop_condition ==
                MoveStopCondition::limit_switch) {
                homing_stopped();
            } else {
                finish_current_move(AckMessageId::position_error);
            }
        }
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

    void homing_stopped() {
        finish_current_move(AckMessageId::stopped_by_condition);
        reset_encoder_pulses();
    }

    auto limit_switch_triggered() -> bool {
        return hardware.check_limit_switch();
    }

    void finish_current_move(
        AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        has_active_move = false;
        uint32_t pulses = 0x0;
        pulses = get_encoder_pulses();
        if (buffered_move.group_id != NO_GROUP) {
            auto ack = Ack{
                .group_id = buffered_move.group_id,
                .seq_id = buffered_move.seq_id,
                .current_position_steps =
                    static_cast<uint32_t>(position_tracker >> 31),
                .encoder_position = pulses,
                .ack_id = ack_msg_id,
            };
            static_cast<void>(
                status_queue_client.send_brushed_move_status_reporter_queue(
                    ack));
        }
        set_buffered_move(BrushedMove{});
    }

    auto get_encoder_pulses() { return hardware.get_encoder_pulses(); }

    void reset_encoder_pulses() { hardware.reset_encoder_pulses(); }

    void set_buffered_move(BrushedMove new_move) { buffered_move = new_move; }

    [[nodiscard]] auto get_buffered_move() const -> BrushedMove {
        return buffered_move;
    }

    bool has_active_move = false;

  private:
    q31_31 position_tracker = 0x0;
    GenericQueue& queue;
    StatusClient& status_queue_client;
    motor_hardware::BrushedMotorHardwareIface& hardware;
    BrushedMove buffered_move = BrushedMove{};
};
}  // namespace brushed_motor_handler
