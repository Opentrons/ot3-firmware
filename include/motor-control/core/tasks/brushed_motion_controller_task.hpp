#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/brushed_motor/brushed_motion_controller.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/utils.hpp"

namespace brushed_motion_controller_task {

using TaskMessage =
    motor_control_task_messages::BrushedMotionControllerTaskMessage;

/**
 * The handler of brushed motion controller messages
 */
template <lms::MotorMechanicalConfig MEConfig,
          can::message_writer_task::TaskClient CanClient>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType =
        brushed_motion_controller::MotionController<MEConfig>;
    MotionControllerMessageHandler(MotorControllerType& controller,
                                   CanClient& can_client)
        : controller{controller}, can_client{can_client} {}
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

    void handle(const can::messages::EnableMotorRequest&) {
        LOG("Received enable motor request");
        controller.enable_motor();
    }

    void handle(const can::messages::DisableMotorRequest&) {
        LOG("Received disable motor request");
        controller.disable_motor();
    }

    void handle(const can::messages::StopRequest&) {
        LOG("Received stop request");
        controller.stop();
    }

    void handle(const can::messages::GripperGripRequest& m) {
        controller.move(m);
    }

    void handle(const can::messages::GripperHomeRequest& m) {
        controller.move(m);
    }

    void handle(const can::messages::ReadLimitSwitchRequest&) {
        auto response = static_cast<uint8_t>(controller.read_limit_switch());
        LOG("Received read limit switch: limit_switch=%d", response);
        can::messages::ReadLimitSwitchResponse msg{{}, response};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle(const can::messages::EncoderPositionRequest&) {
        auto response = controller.read_encoder_pulses();
        LOG("Received read encoder: encoder_pulses=%d", response);
        can::messages::EncoderPositionResponse msg{{}, response};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    brushed_motion_controller::MotionController<MEConfig>& controller;
    CanClient& can_client;
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
              can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(
        brushed_motion_controller::MotionController<MEConfig>* controller,
        CanClient* can_client) {
        auto handler = MotionControllerMessageHandler{*controller, *can_client};
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
