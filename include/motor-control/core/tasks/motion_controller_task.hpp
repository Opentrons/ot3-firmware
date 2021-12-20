#pragma once
#include <variant>

#include "can/core/messages.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motion_controller.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace motion_controller_task {

using TaskMessage = motor_control_task_messages::MotionControlTaskMessage;

/**
 * The message queue message handler.
 */
template <lms::MotorMechanicalConfig MEConfig, typename AllTasks>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType = motion_controller::MotionController<MEConfig>;
    MotionControllerMessageHandler(MotorControllerType& controller, AllTasks & all_tasks)
        : controller{controller}, all_tasks{all_tasks} {}

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void handle(std::monostate m) { static_cast<void>(m); }

    void handle(const can_messages::EnableMotorRequest& m) {
        controller.stop();
    }

    void handle(const can_messages::DisableMotorRequest& m) {
        controller.disable_motor();
    }

    void handle(const can_messages::GetMotionConstraintsRequest& m) {
        auto constraints = controller.get_motion_constraints();
        can_messages::GetMotionConstraintsResponse response_msg{
            .min_velocity = constraints.min_velocity,
            .max_velocity = constraints.max_velocity,
            .min_acceleration = constraints.min_acceleration,
            .max_acceleration = constraints.max_acceleration,
        };
        //        message_writer.write(NodeId::host, response_msg);
    }

    void handle(const can_messages::SetMotionConstraints& m) {
        controller.set_motion_constraints(m);
    }

    void handle(const can_messages::AddLinearMoveRequest& m) {
        controller.move(m);
    }

    MotorControllerType& controller;
    AllTasks & all_tasks;
};

/**
 * The task entry point.
 */
template <lms::MotorMechanicalConfig MEConfig, typename AllTasks>
class MotorControllerTask {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<TaskMessage>;
    MotorControllerTask(QueueType& queue): queue{queue} {}

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(motion_controller::MotionController<MEConfig>* controller, AllTasks* all_tasks) {
        auto handler = MotionControllerMessageHandler{*controller, *all_tasks};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, portMAX_DELAY)) {
                handler.handle_message(message);
            }
        }
    }

  private:
    QueueType& queue;
};

/**
 * Concept describing a class that can message this task.
 * @tparam TaskHolder
 */
template <typename TaskHolder>
concept TaskClient = requires(TaskHolder holder, const TaskMessage& m) {
    {holder.send_motion_controller_queue(m)};
};

}  // namespace motion_controller_task