#pragma once
#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/tasks/tmc_motor_driver_common.hpp"

namespace motion_controller_task {

using TaskMessage = motor_control_task_messages::MotionControlTaskMessage;

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_motion_controller_queue(m)};
};

/**
 * The message queue message handler.
 */
template <lms::MotorMechanicalConfig MEConfig,
          can::message_writer_task::TaskClient CanClient,
          usage_storage_task::TaskClient UsageClient,
          tmc::tasks::TaskClient DriverClient, TaskClient MotionClient>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType = motion_controller::MotionController<MEConfig>;
    MotionControllerMessageHandler(MotorControllerType& controller,
                                   CanClient& can_client,
                                   UsageClient& usage_client,
                                   DriverClient& driver_client,
                                   MotionClient& motion_client,
                                   std::atomic<bool>& diag0_debounced)
        : controller{controller},
          can_client{can_client},
          usage_client{usage_client},
          driver_client{driver_client},
          motion_client{motion_client},
          diag0_debounced{diag0_debounced} {}
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
        if (controller.read_tmc_diag0()) {
            can_client.send_can_message(can::ids::NodeId::host,
                                        can::messages::MotorDriverInErrorState{
                                            .message_index = m.message_index});
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
        if (controller.read_tmc_diag0()) {
            can_client.send_can_message(can::ids::NodeId::host,
                                        can::messages::MotorDriverInErrorState{
                                            .message_index = m.message_index});
        } else {
            controller.move(m);
        }
    }

    void handle(const can::messages::HomeRequest& m) {
        LOG("Motion Controller Received home request: velocity=%d, "
            "groupid=%d, seqid=%d\n",
            m.velocity, m.group_id, m.seq_id);
        if (controller.read_tmc_diag0()) {
            can_client.send_can_message(can::ids::NodeId::host,
                                        can::messages::MotorDriverInErrorState{
                                            .message_index = m.message_index});
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

    // no debouncing needed in final product
    // make messages non-CAN, should be trivial, everything takes std::monostate
    /*
    void handle(const motor_control_task_messages::RouteMotorDriverInterrupt& m)
    { vTaskDelay(pdMS_TO_TICKS(100)); debounce_count++; if (debounce_count > 9)
    { if (controller.read_tmc_diag0()) {
                motion_client.send_motion_controller_queue(motor_control_task_messages::MotorDriverErrorEncountered{.message_index
    = m.message_index}); } else {
                motion_client.send_motion_controller_queue(motor_control_task_messages::ResetMotorDriverErrorHandling{.message_index
    = m.message_index});
            }
            diag0_debounced = false;
            debounce_count = 0;
        } else {
            motion_client.send_motion_controller_queue(motor_control_task_messages::RouteMotorDriverInterrupt{.message_index
    = m.message_index});
        }
    }

    void handle(const motor_control_task_messages::MotorDriverErrorEncountered&
    m) { controller.stop(can::ids::ErrorSeverity::unrecoverable,
                        can::ids::ErrorCode::motor_driver_error_detected);
        if (!controller.is_timer_interrupt_running()) {
            can_client.send_can_message(
                can::ids::NodeId::host,
                can::messages::ErrorMessage{
                    .message_index = m.message_index,
                    .severity = can::ids::ErrorSeverity::unrecoverable,
                    .error_code =
                        can::ids::ErrorCode::motor_driver_error_detected});
            driver_client.send_motor_driver_queue(
                can::messages::ReadMotorDriverErrorStatus{.message_index =
                                                              m.message_index});
        }
    }

    void handle(const
    motor_control_task_messages::ResetMotorDriverErrorHandling& m) {
        static_cast<void>(m);
        controller.clear_cancel_request();
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::ErrorMessage{
                .message_index = m.message_index,
                .severity = can::ids::ErrorSeverity::none,
                .error_code =
                    can::ids::ErrorCode::motor_driver_error_detected}); //
    delete
    }
    */

    void handle(
        const motor_control_task_messages::RouteMotorDriverInterrupt& m) {
        if (debounce_count > 9) {
            if (controller.read_tmc_diag0()) {
                controller.stop(can::ids::ErrorSeverity::unrecoverable,
                                can::ids::ErrorCode::motor_driver_error_detected);
                if (!controller.is_timer_interrupt_running()) {
                    can_client.send_can_message(
                        can::ids::NodeId::host,
                        can::messages::ErrorMessage{
                            .message_index = m.message_index,
                            .severity = can::ids::ErrorSeverity::unrecoverable,
                            .error_code =
                                can::ids::ErrorCode::motor_driver_error_detected});
                    driver_client.send_motor_driver_queue(
                        can::messages::ReadMotorDriverErrorStatusRequest{
                            .message_index = m.message_index});
                }
            } else {
                controller.clear_cancel_request();
            }
            diag0_debounced = false;
            debounce_count = 0;
        } else {
            debounce_count++;
            motion_client.send_motion_controller_queue(motor_control_task_messages::RouteMotorDriverInterrupt{.message_index = m.message_index});
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }

    MotorControllerType& controller;
    CanClient& can_client;
    UsageClient& usage_client;
    DriverClient& driver_client;
    MotionClient& motion_client;
    std::atomic<bool>& diag0_debounced;
    std::atomic<uint8_t> debounce_count = 0;
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
              can::message_writer_task::TaskClient CanClient,
              usage_storage_task::TaskClient UsageClient,
              tmc::tasks::TaskClient DriverClient, TaskClient MotionClient>
    [[noreturn]] void operator()(
        motion_controller::MotionController<MEConfig>* controller,
        CanClient* can_client, UsageClient* usage_client,
        DriverClient* driver_client, MotionClient* motion_client) {
        auto handler = MotionControllerMessageHandler{
            *controller,    *can_client,    *usage_client,
            *driver_client, *motion_client, diag0_debounced};
        TaskMessage message{};
        bool first_run = true;
        for (;;) {
            if (first_run && controller->engage_at_startup) {
                controller->enable_motor();
                first_run = false;
            }
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

    /*
    void run_diag0_interrupt() {
        if (!diag0_debounced) {
            static_cast<void>(queue.try_write_isr(motor_control_task_messages::RouteMotorDriverInterrupt{.message_index
    = 0})); diag0_debounced = true;
        }
    }

    void run_diag0_interrupt() {
        if (controller->read_tmc_diag0()) {
            controller->stop(can::ids::ErrorSeverity::unrecoverable,
                            can::ids::ErrorCode::motor_driver_error_detected);
            if (!controller->is_timer_interrupt_running()) {
                can_client->send_can_message(
                    can::ids::NodeId::host,
                    can::messages::ErrorMessage{
                        .message_index = 0,
                        .severity = can::ids::ErrorSeverity::unrecoverable,
                        .error_code =
                            can::ids::ErrorCode::motor_driver_error_detected});
                driver_client->send_motor_driver_queue(
                    can::messages::ReadMotorDriverErrorStatus{.message_index =
    0});
            }
        } else {
            controller->clear_cancel_request();
        }
    }
    */

    void run_diag0_interrupt() {
        if (!diag0_debounced){
            static_cast<void>(queue.try_write_isr(motor_control_task_messages::RouteMotorDriverInterrupt{.message_index = 0}));
            diag0_debounced = true;
        }
    }

  private:
    QueueType& queue;
    std::atomic<bool> diag0_debounced = false;
    // static motion_controller::MotionController<lms::MotorMechanicalConfig>*
    // controller = nullptr; static can::message_writer_task::TaskClient*
    // can_client = nullptr; static tmc::tasks::TaskClient* driver_client =
    // nullptr; static member object for motion controller (a nullable pointer)
    // Can that much calling of motion controller be done in interrupt??
    // do what impacts the least
};

}  // namespace motion_controller_task
