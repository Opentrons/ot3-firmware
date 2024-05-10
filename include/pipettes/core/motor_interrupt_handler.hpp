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

  private:
    SensorClient& sensor_client;
};
}  // namespace pipettes