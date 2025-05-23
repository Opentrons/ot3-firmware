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
          can::message_writer_task::TaskClient CanClient,
          usage_storage_task::TaskClient UsageClient>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType = motion_controller::MotionController<MEConfig>;
    MotionControllerMessageHandler(MotorControllerType& controller,
                                   CanClient& can_client,
                                   UsageClient& usage_client,
                                   uint16_t evo_disp_count_key)
        : controller{controller},
          can_client{can_client},
          usage_client{usage_client},
          evo_disp_count_key{evo_disp_count_key} {}
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

    void handle(const can::messages::StopRequest& m) {
        LOG("Received stop request");
        controller.stop();
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::EnableMotorRequest& m) {
        LOG("Received enable motor request");
        if (controller.check_tmc_diag0()) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ErrorMessage{
                    .message_index = m.message_index,
                    .severity = can::ids::ErrorSeverity::unrecoverable,
                    .error_code =
                        can::ids::ErrorCode::motor_driver_error_detected});
        } else {
            controller.enable_motor();
            can_client.send_can_message(can::ids::NodeId::host,
                                        can::messages::ack_from_request(m));
        }
    }

    void handle(const can::messages::DisableMotorRequest& m) {
        LOG("Received disable motor request");
        controller.disable_motor();
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::GetMotionConstraintsRequest& m) {
        auto constraints = controller.get_motion_constraints();
        can::messages::GetMotionConstraintsResponse response_msg{
            .message_index = m.message_index,
            .min_velocity = constraints.min_velocity,
            .max_velocity = constraints.max_velocity,
            .min_acceleration = constraints.min_acceleration,
            .max_acceleration = constraints.max_acceleration,
        };
        LOG("Received get motion constraints request");
        can_client.send_can_message(can::ids::NodeId::host, response_msg);
    }

    void handle(const can::messages::SetMotionConstraints& m) {
        LOG("Received set motion constraints: minvel=%d, maxvel=%d, minacc=%d, "
            "maxacc=%d",
            m.min_velocity, m.max_velocity, m.min_acceleration,
            m.max_acceleration);
        controller.set_motion_constraints(m);
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::AddLinearMoveRequest& m) {
        LOG("Received add linear move request: velocity=%d, acceleration=%d, "
            "groupid=%d, seqid=%d, duration=%d, stopcondition=%d",
            m.velocity, m.acceleration, m.group_id, m.seq_id, m.duration,
            m.request_stop_condition);
        if (controller.check_tmc_diag0()) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ErrorMessage{
                    .message_index = m.message_index,
                    .severity = can::ids::ErrorSeverity::unrecoverable,
                    .error_code =
                        can::ids::ErrorCode::motor_driver_error_detected});
        } else {
            controller.move(m);
        }
    }

#ifdef USE_SENSOR_MOVE
    void handle(const can::messages::AddSensorMoveRequest& m) {
        LOG("Received add linear move request: velocity=%d, acceleration=%d, "
            "groupid=%d, seqid=%d, duration=%d, stopcondition=%d",
            m.velocity, m.acceleration, m.group_id, m.seq_id, m.duration,
            m.request_stop_condition);
        if (controller.check_tmc_diag0()) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ErrorMessage{
                    .message_index = m.message_index,
                    .severity = can::ids::ErrorSeverity::unrecoverable,
                    .error_code =
                        can::ids::ErrorCode::motor_driver_error_detected});
        } else {
            controller.move(m);
        }
    }
#endif
    void handle(const can::messages::IncreaseEvoDispenseRequest& m) {
        if (evo_disp_count_key == 0) {
            return;
        }
        auto message =
            usage_messages::IncreaseErrorCount{.key = evo_disp_count_key};
        usage_client.send_usage_storage_queue(message);
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::HomeRequest& m) {
        LOG("Motion Controller Received home request: velocity=%d, "
            "groupid=%d, seqid=%d\n",
            m.velocity, m.group_id, m.seq_id);
        if (controller.check_tmc_diag0()) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ErrorMessage{
                    .message_index = m.message_index,
                    .severity = can::ids::ErrorSeverity::unrecoverable,
                    .error_code =
                        can::ids::ErrorCode::motor_driver_error_detected});
        } else {
            controller.move(m);
        }
    }

    void handle(const can::messages::ReadLimitSwitchRequest& m) {
        auto response = static_cast<uint8_t>(controller.read_limit_switch());
        LOG("Received read limit switch: limit_switch=%d", response);
        can::messages::ReadLimitSwitchResponse msg{
            .message_index = m.message_index, .switch_status = response};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle(const can::messages::MotorPositionRequest& m) {
        auto position = controller.read_motor_position();
        auto encoder = controller.read_encoder_pulses();
        auto flags = controller.get_position_flags();
        LOG("Received read encoder: position=%d encoder_pulses=%d flags=0x%2X",
            position, encoder, flags);
        can::messages::MotorPositionResponse msg{
            .message_index = m.message_index,
            .current_position = position,
            .encoder_position = encoder,
            .position_flags = flags};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    void handle(const can::messages::UpdateMotorPositionEstimationRequest& m) {
        if (!controller.update_position(m)) {
            // If the motor controller can't ask the interrupt handler to
            // handle the message, we respond with the current status as-is.
            can::messages::UpdateMotorPositionEstimationResponse response{
                .message_index = m.message_index,
                .current_position = controller.read_motor_position(),
                .encoder_position = controller.read_encoder_pulses(),
                .position_flags = controller.get_position_flags()};
            can_client.send_can_message(can::ids::NodeId::host, response);
        }
    }
    void handle(const can::messages::GetMotorUsageRequest& m) {
        controller.send_usage_data(m.message_index, usage_client);
    }

    void handle(const can::messages::MotorStatusRequest& m) {
        auto response = static_cast<uint8_t>(controller.is_motor_enabled());
        can::messages::MotorStatusResponse msg{.message_index = m.message_index,
                                               .enabled = response};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    MotorControllerType& controller;
    CanClient& can_client;
    UsageClient& usage_client;
    uint16_t evo_disp_count_key;
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
    MotionControllerTask(QueueType& queue, uint16_t evo_disp_count_key)
        : queue{queue}, evo_disp_count_key{evo_disp_count_key} {}
    MotionControllerTask(QueueType& queue)
        : queue{queue}, evo_disp_count_key{0} {}
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
        motion_controller::MotionController<MEConfig>* controller,
        CanClient* can_client, UsageClient* usage_client) {
        auto handler = MotionControllerMessageHandler{
            *controller, *can_client, *usage_client, evo_disp_count_key};
        TaskMessage message{};
        bool first_run = true;
        for (;;) {
            if (first_run && controller->disengage_at_startup) {
                controller->disable_motor();
                first_run = false;
            }
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

  private:
    QueueType& queue;
    uint16_t evo_disp_count_key;
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
