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
    using InterruptHandler = motor_handler::MotorInterruptHandler<
        freertos_message_queue::FreeRTOSMessageQueue, StatusClient,
        MotorMoveMessage, MotorHardware>;
    using UpdatePositionQueue =
        QueueImpl<can::messages::UpdateMotorPositionEstimationRequest>;
    PipetteMotorInterruptHandler() = delete;
    PipetteMotorInterruptHandler(
        MoveQueue& incoming_move_queue, StatusClient& outgoing_queue,
        MotorHardware& hardware_iface, stall_check::StallCheck& stall,
        UpdatePositionQueue& incoming_update_position_queue,
        SensorClient& sensor_queue_client) :
          InterruptHandler(incoming_move_queue, outgoing_queue, hardware_iface,
                           stall, incoming_update_position_queue),
          sensor_client(sensor_queue_client) {
//        sensor_client = sensor_tasks::QueueClient{};
        }

    ~PipetteMotorInterruptHandler() = default;

    auto operator=(PipetteMotorInterruptHandler&)
        -> PipetteMotorInterruptHandler& = delete;

    auto operator=(PipetteMotorInterruptHandler&&)
        -> PipetteMotorInterruptHandler&& = delete;

    PipetteMotorInterruptHandler(PipetteMotorInterruptHandler&) = delete;

    PipetteMotorInterruptHandler(PipetteMotorInterruptHandler&&) = delete;

    // can determine which queue write function to use with the sensor id that gets passed in
    // send a message to sensor queue inside update_move, if _has_active_move

    // need to write a new kind of move that contains the stuff thats gonna be sent in the new can message

    void update_move() {
        // do stuff in here
        _has_active_move = move_queue.try_read_isr(&buffered_move);
        if (_has_active_move) {
            hardware.enable_encoder();
            buffered_move.start_encoder_position =
                    hardware.get_encoder_pulses();
        }
        if (set_direction_pin()) {
            hardware.positive_direction();
        } else {
            hardware.negative_direction();
        }
        if (_has_active_move && buffered_move.check_stop_condition(
                MoveStopCondition::limit_switch)) {
            position_tracker = 0x7FFFFFFFFFFFFFFF;
            update_hardware_step_tracker();
            hardware.position_flags.clear_flag(
                    can::ids::MotorPositionFlags::stepper_position_ok);
            hardware.position_flags.clear_flag(
                    can::ids::MotorPositionFlags::encoder_position_ok);
        }
    }

    void finish_current_move(
            AckMessageId ack_msg_id = AckMessageId::complete_without_condition) {
        _has_active_move = false;
        tick_count = 0x0;
        stall_handled = false;
        build_and_send_ack(ack_msg_id);
        set_buffered_move(MotorMoveMessage{});
        // update the stall check ideal encoder counts based on
        // last known location

        // check if queue is empty in here and send bindsensoroutputrequest

        if (!has_move_messages()) {
            stall_checker.reset_itr_counts(hardware.get_step_tracker());
        }
    }



  private:
    SensorClient& sensor_client;
};
}  // namespace pipettes