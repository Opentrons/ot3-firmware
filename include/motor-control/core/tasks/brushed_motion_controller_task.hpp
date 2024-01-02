#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/brushed_motor/brushed_motion_controller.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "motor-control/core/utils.hpp"

namespace brushed_motion_controller_task {

using TaskMessage =
    motor_control_task_messages::BrushedMotionControllerTaskMessage;

/**
 * The handler of brushed motion controller messages
 */
template <lms::MotorMechanicalConfig MEConfig,
          can::message_writer_task::TaskClient CanClient,
          usage_storage_task::TaskClient UsageClient>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType =
        brushed_motion_controller::MotionController<MEConfig>;
    MotionControllerMessageHandler(MotorControllerType& controller,
                                   CanClient& can_client,
                                   UsageClient& usage_client)
        : controller{controller},
          can_client{can_client},
          usage_client{usage_client} {}
    MotionControllerMessageHandler(const MotionControllerMessageHandler& c) =
        delete;
    MotionControllerMessageHandler(const MotionControllerMessageHandler&& c) =
        delete;
    auto operator=(const MotionControllerMessageHandler& c) = delete;
    auto operator=(const MotionControllerMessageHandler&& c) = delete;
    ~MotionControllerMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void handle(std::monostate&) {}

    void handle(const can::messages::EnableMotorRequest& m) {
        LOG("Received enable motor request");
        controller.enable_motor();
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::DisableMotorRequest& m) {
        LOG("Received disable motor request");
        controller.disable_motor();
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::StopRequest& m) {
        LOG("Received stop request");
        controller.stop();
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::AddBrushedLinearMoveRequest& m) {
        controller.move(m);
    }

    void handle(const can::messages::GripperGripRequest& m) {
        controller.move(m);
    }

    void handle(const can::messages::GripperHomeRequest& m) {
        controller.move(m);
    }

    void handle(const can::messages::ReadLimitSwitchRequest& m) {
        auto response = static_cast<uint8_t>(controller.read_limit_switch());
        LOG("Received read limit switch: limit_switch=%d", response);
        can::messages::ReadLimitSwitchResponse msg{
            .message_index = m.message_index, .switch_status = response};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle(const can::messages::MotorPositionRequest& m) {
        auto response = controller.read_encoder_pulses();
        auto flags = controller.get_position_flags();
        LOG("Received read encoder: encoder_pulses=%d flags=0x%2X", response,
            flags);
        can::messages::MotorPositionResponse msg{
            .message_index = m.message_index,
            .current_position = 0,
            .encoder_position = response,
            .position_flags = flags};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle(const can::messages::SetGripperErrorToleranceRequest& m) {
        controller.set_error_tolerance(m);
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::GetMotorUsageRequest& m) {
        controller.send_usage_data(m.message_index, usage_client);
    }

    void handle(const can::messages::GripperJawStateRequest& m) {
        auto jaw_state = controller.get_jaw_state();
        can::messages::GripperJawStateResponse msg{
            .message_index = m.message_index,
            .jaw_state = static_cast<uint8_t>(jaw_state)};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle(const can::messages::SetGripperJawHoldoffRequest& m) {
        controller.set_idle_holdoff(m);
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::GripperJawHoldoffRequest& m) {
        auto holdoff = controller.get_idle_holdoff_ms();
        can::messages::GripperJawHoldoffResponse msg{
            .message_index = m.message_index, .holdoff_ms = holdoff};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    brushed_motion_controller::MotionController<MEConfig>& controller;
    CanClient& can_client;
    UsageClient& usage_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MotionControllerTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    MotionControllerTask(QueueType& queue) : queue{queue} {}
    MotionControllerTask(const MotionControllerTask& c) = delete;
    MotionControllerTask(const MotionControllerTask&& c) = delete;
    auto operator=(const MotionControllerTask& c) = delete;
    auto operator=(const MotionControllerTask&& c) = delete;
    ~MotionControllerTask() = default;

    /**
     * Task entry point.
     */
    template <lms::MotorMechanicalConfig MEConfig,
              can::message_writer_task::TaskClient CanClient,
              usage_storage_task::TaskClient UsageClient>
    [[noreturn]] void operator()(
        brushed_motion_controller::MotionController<MEConfig>* controller,
        CanClient* can_client, UsageClient* usage_client) {
        auto handler = MotionControllerMessageHandler{*controller, *can_client,
                                                      *usage_client};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_brushed_motion_controller_queue(m)};
};

}  // namespace brushed_motion_controller_task
