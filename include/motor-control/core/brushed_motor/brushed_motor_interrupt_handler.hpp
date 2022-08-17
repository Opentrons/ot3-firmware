#pragma once

#include <algorithm>
#include <atomic>

#include "common/core/logging.h"
#include "common/core/message_queue.hpp"
#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/core/tasks/move_status_reporter_task.hpp"
#include "ot_utils/core/pid.hpp"

namespace brushed_motor_handler {

using namespace motor_messages;

/*
 *
 * A brushed motor motion handler class.
 *
 */

static constexpr int32_t ACCEPTABLE_POSITION_ERROR = 10;

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

    auto controlled_move_to(int32_t encoder_position) -> int32_t {
        int32_t move_delta = hardware.get_encoder_pulses() - encoder_position;
        if (!controlling) {
            if (move_delta < 0) {
                hardware.positive_direction();
            } else if (move_delta > 0) {
                hardware.negative_direction();
            }
            controlling = true;
        }
        if (std::abs(move_delta) < ACCEPTABLE_POSITION_ERROR) {
            current_control_pwm = 0;
            driver_hardware.update_pwm_settings(current_control_pwm);
            controlling = false;
        } else {
            // the compute the pid with the abs of move_delta because the pwm value
            // is always positive regardless of moving forward or backward
            double pid_output = hardware.update_control(std::abs(move_delta));
            // Floor the PWM to 3 so that we still move when the pid output is so
            // low that it would otherwise round to 0 since we
            uint32_t pid_pwm_output = std::clamp(int(pid_output), 3, 100);
            if (pid_pwm_output != current_control_pwm) {
                current_control_pwm = pid_pwm_output;
                driver_hardware.update_pwm_settings(current_control_pwm);
            }
        }
        return move_delta;
    }

    void run_interrupt() {
        if (!has_active_move && has_messages()) {
            update_and_start_move();
        }
        if (has_active_move) {
            switch (buffered_move.stop_condition) {
                // homing move
                case MoveStopCondition::limit_switch:
                    if (limit_switch_triggered()) {
                        homing_stopped();
                    }
                    break;
                // linear move
                case MoveStopCondition::encoder_position:
                    if (std::abs(controlled_move_to(
                            buffered_move.encoder_position)) <
                        ACCEPTABLE_POSITION_ERROR) {
                        finish_current_move(AckMessageId::stopped_by_condition);
                    }
                    break;
                // grip move
                case MoveStopCondition::none:
                    if (is_sensing() && is_idle) {
                        finish_current_move(
                            AckMessageId::complete_without_condition);
                    }
                    break;
                case MoveStopCondition::cap_sensor:
                    // TODO write cap sensor move code
                    break;
            }
        } else if (holding) {
            controlled_move_to(hold_encoder_position);
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
        hardware.reset_control();
        holding = false;
        switch (buffered_move.stop_condition) {
            case MoveStopCondition::limit_switch:
                hardware.ungrip();
                break;
            case MoveStopCondition::encoder_position:
                if (hardware.get_encoder_pulses() ==
                    buffered_move.encoder_position) {
                    LOG("Attempting to add a gripper move to our current "
                        "position");
                    finish_current_move(AckMessageId::stopped_by_condition);
                    break;
                }
                // multiplied by -1 here because the "error" is the opposite
                // direction we're going
                LOG("Brushed motor moving %i µm",
                    -1 * controlled_move_to(buffered_move.encoder_position));
                break;
            case MoveStopCondition::none:
                hardware.grip();
                break;
            case MoveStopCondition::cap_sensor:
                // TODO write cap sensor move code
                break;
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
        // if we were instructed to move to particular spot maintain that
        // position otherwise we want to continue to apply pressure
        if (buffered_move.stop_condition ==
            MoveStopCondition::encoder_position) {
            hold_encoder_position = buffered_move.encoder_position;
            holding = true;
        }
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
    std::atomic<bool> holding = false;
    int32_t hold_encoder_position = 0;
    uint32_t current_control_pwm = 0;
    std::atomic<bool> controlling = false;
};
}  // namespace brushed_motor_handler
