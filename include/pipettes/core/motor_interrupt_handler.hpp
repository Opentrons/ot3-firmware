#pragma once

#include "motor-control/core/stepper_motor/motor_interrupt_handler.hpp"
#include "pipettes/core/sensor_tasks.hpp"

namespace pipettes {
using namespace motor_messages;
template <template <class> class QueueImpl, class StatusClient,
          typename MotorMoveMessage, typename MotorHardware, class SensorClient>
requires MessageQueue<QueueImpl<MotorMoveMessage>, MotorMoveMessage> &&
    std::is_base_of_v<motor_hardware::MotorHardwareIface, MotorHardware>
class PipetteMotorInterruptHandler
    : public motor_handler::MotorInterruptHandler<
          freertos_message_queue::FreeRTOSMessageQueue, StatusClient,
          MotorMoveMessage, MotorHardware> {
  public:
    using MoveQueue = QueueImpl<MotorMoveMessage>;
    using UpdatePositionQueue =
        QueueImpl<can::messages::UpdateMotorPositionEstimationRequest>;
    PipetteMotorInterruptHandler() = delete;
    PipetteMotorInterruptHandler(
        MoveQueue& incoming_move_queue, StatusClient& outgoing_queue,
        MotorHardware& hardware_iface, stall_check::StallCheck& stall,
        UpdatePositionQueue& incoming_update_position_queue,
        SensorClient& sensor_queue_client)
        : motor_handler::MotorInterruptHandler<
              freertos_message_queue::FreeRTOSMessageQueue, StatusClient,
              MotorMoveMessage, MotorHardware>(
              incoming_move_queue, outgoing_queue, hardware_iface, stall,
              incoming_update_position_queue),
          sensor_client(sensor_queue_client) {}

    ~PipetteMotorInterruptHandler() = default;

    auto operator=(PipetteMotorInterruptHandler&)
        -> PipetteMotorInterruptHandler& = delete;

    auto operator=(PipetteMotorInterruptHandler&&)
        -> PipetteMotorInterruptHandler&& = delete;

    PipetteMotorInterruptHandler(PipetteMotorInterruptHandler&) = delete;

    PipetteMotorInterruptHandler(PipetteMotorInterruptHandler&&) = delete;

    void run_interrupt() {
        // handle various error states
        if (this->clear_queue_until_empty) {
            // If we were executing a move when estop asserted, and
            // what's in the queue is the remaining enqueued moves from
            // that group, then we will have called
            // cancel_and_clear_moves and clear_queue_until_empty will
            // be true. That means we should pop out the queue.
            // clear_queue_until_empty will also be true if we were in
            // the steady-state estop asserted; got new messages; and
            // the other branch of this if asserted. In either case, an
            // error message has been sent, so we need to just keep
            // clearing the queue.
            this->clear_queue_until_empty = this->pop_and_discard_move();
        } else if (this->in_estop) {
            this->handle_update_position_queue();
            this->in_estop = this->estop_update();
        } else if (this->estop_triggered()) {
            this->cancel_and_clear_moves(can::ids::ErrorCode::estop_detected);
            this->in_estop = true;
        } else if (this->hardware.has_cancel_request()) {
            this->cancel_and_clear_moves(can::ids::ErrorCode::stop_requested,
                                         can::ids::ErrorSeverity::warning);
        } else {
            // Normal Move logic
            this->run_normal_interrupt();
        }
    }

    void handle_move_type(motor_messages::Move m) {}

    void handle_move_type(motor_messages::SensorSyncMove m) {
        auto binding = static_cast<can::ids::SensorOutputBinding>(0x3);
        auto msg = can::messages::BindSensorOutputRequest{
                .message_index = m.message_index,
                .sensor = can::ids::SensorType::pressure,
                .sensor_id = m.sensor_id,
                .binding = binding
        };
        send_to_pressure_sensor_queue(&msg);
    }

    void update_move() {
        this->_has_active_move =
            this->move_queue.try_read_isr(&this->buffered_move);
        if (this->_has_active_move) {
            handle_move_type(this->buffered_move);
            this->hardware.enable_encoder();
            this->buffered_move.start_encoder_position =
                this->hardware.get_encoder_pulses();
        }
        if (this->set_direction_pin()) {
            this->hardware.positive_direction();
        } else {
            this->hardware.negative_direction();
        }
        if (this->_has_active_move && this->buffered_move.check_stop_condition(
                                          MoveStopCondition::limit_switch)) {
            this->position_tracker = 0x7FFFFFFFFFFFFFFF;
            this->update_hardware_step_tracker();
            this->hardware.position_flags.clear_flag(
                can::ids::MotorPositionFlags::stepper_position_ok);
            this->hardware.position_flags.clear_flag(
                can::ids::MotorPositionFlags::encoder_position_ok);
        }
    }

    void finish_current_move(
        AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        this->_has_active_move = false;
        this->tick_count = 0x0;
        this->stall_handled = false;
        this->build_and_send_ack(
            ack_msg_id);
        (MotorMoveMessage{});
        // update the stall check ideal encoder counts based on
        // last known location

        if (!this->has_move_messages()) {
            this->stall_checker.reset_itr_counts(
                this->hardware.get_step_tracker());
            auto& stop_msg = can::messages::BindSensorOutputRequest{
                .message_index = this->buffered_move.message_index,
                .sensor = can::ids::SensorType::pressure,
                .sensor_id = this->buffered_move.sensor_id,
                .binding = can::ids::SensorOutputBinding::none};
            send_to_pressure_sensor_queue(&stop_msg);
        }
    }

    void send_to_pressure_sensor_queue(can::messages::SensorOutputBinding& m) {
        if (this->buffered_move.sensor_id == can::ids::SensorId::S1) {
            sensor_client.send_pressure_sensor_queue_front(m);
        } else {
            sensor_client.send_pressure_sensor_queue_rear(m);
        }
    }

  private:
    SensorClient& sensor_client;
};
}  // namespace pipettes