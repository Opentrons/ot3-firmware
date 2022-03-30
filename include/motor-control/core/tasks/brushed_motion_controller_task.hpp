#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/utils.hpp"

namespace brushed_motion_controller_task {

using TaskMessage =
    motor_control_task_messages::BrushedMotionControllerTaskMessage;

/**
 * The handler of brushed motion controller messages
 */
template <message_writer_task::TaskClient CanClient>
class MotionControllerMessageHandler {
  public:
    MotionControllerMessageHandler(
        motor_hardware::BrushedMotorHardwareIface& motor, CanClient& can_client)
        : motor{motor}, can_client{can_client} {}
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
    void handle(std::monostate m) { static_cast<void>(m); }

    void handle(const can_messages::GripperGripRequest& m) { motor.grip(); }

    void handle(const can_messages::GripperHomeRequest& m) { motor.home(); }

    motor_hardware::BrushedMotorHardwareIface& motor;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl,
          message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MotionControllerTask {
  public:
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
    [[noreturn]] void operator()(
        motor_hardware::BrushedMotorHardwareIface* motor,
        CanClient* can_client) {
        auto handler = MotionControllerMessageHandler{*motor, *can_client};
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