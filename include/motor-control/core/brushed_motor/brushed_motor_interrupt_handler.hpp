#pragma once

#include <atomic>

#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "motor-control/core/brushed_motor/driver_interface.hpp"
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

static constexpr uint32_t HOLDOFF_TICKS =
    32;  // hold off for 1 ms (with a 32k Hz timer)

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
        brushed_motor_driver::BrushedMotorDriverIface& driver_iface)
        : queue(incoming_queue),
          status_queue_client(outgoing_queue),
          hardware(hardware_iface),
          driver_hardware(driver_iface) {}
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
            update_and_start_move();
        }
        if (has_active_move) {
            if (buffered_move.stop_condition ==
                    MoveStopCondition::limit_switch &&
                limit_switch_triggered()) {
                homing_stopped();
            } else {
                if (is_sensing() && is_idle) {
                    finish_current_move(
                        AckMessageId::complete_without_condition);
                }
            }
        }
    }

    auto is_sensing() -> bool {
        /* When gripping, hold off a certain number of ticks before checking if
         * the encoder is idle */
        if (buffered_move.stop_condition == MoveStopCondition::limit_switch) {
            return false;
        }
        if (tick < HOLDOFF_TICKS) {
            tick++;
            return false;
        }
        return true;
    }

    void set_enc_idle_state(bool val) {
        /* When the encoder speed timer overflows, this means the encoder is
         * moving at a frequency lower than 8 Hz, we can assume the gripper jaw
         * has stopped moving and the active move is complete.
         */
        is_idle = val;
    }

    void update_and_start_move() {
        tick = 0;
        has_active_move = queue.try_read_isr(&buffered_move);
        if (buffered_move.duty_cycle != 0U) {
            driver_hardware.update_pwm_settings(buffered_move.duty_cycle);
        }
        if (buffered_move.stop_condition == MoveStopCondition::limit_switch) {
            hardware.ungrip();
        } else {
            hardware.grip();
        }
    }

    void homing_stopped() {
        hardware.reset_encoder_pulses();
        finish_current_move(AckMessageId::stopped_by_condition);
    }

    auto limit_switch_triggered() -> bool {
        return hardware.check_limit_switch();
    }

    void finish_current_move(
        AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        has_active_move = false;
        if (buffered_move.group_id != NO_GROUP) {
            auto ack = buffered_move.build_ack(hardware.get_encoder_pulses(),
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

    bool has_active_move = false;
    std::atomic<bool> is_idle = true;
    uint32_t tick = 0;

  private:
    GenericQueue& queue;
    StatusClient& status_queue_client;
    motor_hardware::BrushedMotorHardwareIface& hardware;
    brushed_motor_driver::BrushedMotorDriverIface& driver_hardware;
    BrushedMove buffered_move = BrushedMove{};
};
}  // namespace brushed_motor_handler
