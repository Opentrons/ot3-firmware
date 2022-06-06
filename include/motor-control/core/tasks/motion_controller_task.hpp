#pragma once
#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace motion_controller_task {

using TaskMessage = motor_control_task_messages::MotionControlTaskMessage;

/**
 * The message queue message handler.
 */
template <lms::MotorMechanicalConfig MEConfig,
          message_writer_task::TaskClient CanClient>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType = motion_controller::MotionController<MEConfig>;
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

    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void handle(std::monostate m) { static_cast<void>(m); }

    void handle(const can_messages::StopRequest&) {
        LOG("Received stop request");
        controller.stop();
    }

    void handle(const can_messages::EnableMotorRequest&) {
        LOG("Received enable motor request");
        controller.enable_motor();
    }

    void handle(const can_messages::DisableMotorRequest&) {
        LOG("Received disable motor request");
        controller.disable_motor();
    }

    void handle(const can_messages::GetMotionConstraintsRequest&) {
        auto constraints = controller.get_motion_constraints();
        can_messages::GetMotionConstraintsResponse response_msg{
            .min_velocity = constraints.min_velocity,
            .max_velocity = constraints.max_velocity,
            .min_acceleration = constraints.min_acceleration,
            .max_acceleration = constraints.max_acceleration,
        };
        LOG("Received get motion constraints request");
        can_client.send_can_message(can_ids::NodeId::host, response_msg);
    }

    void handle(const can_messages::SetMotionConstraints& m) {
        LOG("Received set motion constraints: minvel=%d, maxvel=%d, minacc=%d, "
            "maxacc=%d",
            m.min_velocity, m.max_velocity, m.min_acceleration,
            m.max_acceleration);
        controller.set_motion_constraints(m);
    }

    void handle(const can_messages::AddLinearMoveRequest& m) {
        LOG("Received add linear move request: velocity=%d, acceleration=%d, "
            "groupid=%d, seqid=%d, duration=%d, stopcondition=%d",
            m.velocity, m.acceleration, m.group_id, m.seq_id, m.duration,
            m.request_stop_condition);
        controller.move(m);
    }

    void handle(const can_messages::HomeRequest& m) {
        LOG("Motion Controller Received home request: velocity=%d, "
            "groupid=%d, seqid=%d\n",
            m.velocity, m.group_id, m.seq_id);
        controller.move(m);
    }

    void handle(const can_messages::ReadLimitSwitchRequest&) {
        auto response = static_cast<uint8_t>(controller.read_limit_switch());
        LOG("Received read limit switch: limit_switch=%d", response);
        can_messages::ReadLimitSwitchResponse msg{{}, response};
        can_client.send_can_message(can_ids::NodeId::host, msg);
    }

    void handle(const can_messages::EncoderPositionRequest&) {
        auto response = static_cast<int32_t>(controller.read_encoder_pulses());
        LOG("Received read encoder: encoder_pulses=%d", response);
        can_messages::EncoderPositionResponse msg{{}, response};
        can_client.send_can_message(can_ids::NodeId::host, msg);
    }

    MotorControllerType& controller;
    CanClient& can_client;
};

/**
 * The task entry point.
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
              message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(
        motion_controller::MotionController<MEConfig>* controller,
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
    {client.send_motion_controller_queue(m)};
};

}  // namespace motion_controller_task
