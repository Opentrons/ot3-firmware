#pragma once
#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/stepper_motor/motion_controller.hpp"
#include "motor-control/core/tasks/tmc_motor_driver_common.hpp"
#include "motor-control/core/tasks/usage_storage_task.hpp"
#include "pipettes/core/tasks/messages.hpp"

namespace pipettes {

namespace tasks {

namespace motion_controller_task {

using TaskMessage = pipettes::task_messages::motor_control_task_messages::
    MotionControlTaskMessage;

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
          tmc::tasks::GearTaskClient DriverClient, TaskClient MotionClient>
class MotionControllerMessageHandler {
  public:
    using MotorControllerType =
        pipette_motion_controller::PipetteMotionController<MEConfig>;
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

    void handle(const can::messages::GearEnableMotorRequest& m) {
        LOG("Received enable motor request");
        // TODO only toggle the enable pin once since all motors share
        // a single enable pin line.
        if (controller.read_tmc_diag0()) {
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

    void handle(const can::messages::GearDisableMotorRequest& m) {
        LOG("Received disable motor request");
        // TODO only toggle the enable pin once since all motors share
        // a single enable pin line.
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

    void handle(const can::messages::TipActionRequest& m) {
        LOG("Motion Controller Received a tip action request: velocity=%d, "
            "acceleration=%d, groupid=%d, seqid=%d\n",
            m.velocity, m.acceleration, m.group_id, m.seq_id);
        if (controller.read_tmc_diag0()) {
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

    void handle(const can::messages::GetMotorUsageRequest& m) {
        controller.send_usage_data(m.message_index, usage_client);
    }

    void handle(const pipettes::task_messages::motor_control_task_messages::
                    RouteMotorDriverInterrupt& m) {
        if (m.debounce_count > 9) {
            if (controller.read_tmc_diag0()) {
                controller.stop(
                    can::ids::ErrorSeverity::unrecoverable,
                    can::ids::ErrorCode::motor_driver_error_detected);
                if (!controller.is_timer_interrupt_running()) {
                    can_client.send_can_message(
                        can::ids::NodeId::host,
                        can::messages::ErrorMessage{
                            .message_index = m.message_index,
                            .severity = can::ids::ErrorSeverity::unrecoverable,
                            .error_code = can::ids::ErrorCode::
                                motor_driver_error_detected});
                    driver_client.send_motor_driver_queue(
                        can::messages::ReadMotorDriverErrorStatusRequest{
                            .message_index = m.message_index});
                }
            }
            diag0_debounced = false;
        } else {
            vTaskDelay(pdMS_TO_TICKS(100));
            motion_client.send_motion_controller_queue(
                increment_message_debounce_count(m));
        }
    }

    auto increment_message_debounce_count(
        const pipettes::task_messages::motor_control_task_messages::
            RouteMotorDriverInterrupt& m) -> pipettes::task_messages::
        motor_control_task_messages::RouteMotorDriverInterrupt {
        return pipettes::task_messages::motor_control_task_messages::
            RouteMotorDriverInterrupt{
                .message_index = m.message_index,
                .debounce_count = static_cast<uint8_t>(m.debounce_count + 1)};
    }
    
    void handle(const can::messages::MotorStatusRequest& m) {
        auto response = static_cast<uint8_t>(controller.is_motor_enabled());
        can::messages::GearMotorStatusResponse msg{
            .message_index = m.message_index, .enabled = response};
        can_client.send_can_message(can::ids::NodeId::host, msg);
    }

    MotorControllerType& controller;
    CanClient& can_client;
    UsageClient& usage_client;
    DriverClient& driver_client;
    MotionClient& motion_client;
    std::atomic<bool>& diag0_debounced;
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
              tmc::tasks::GearTaskClient DriverClient, TaskClient MotionClient>
    [[noreturn]] void operator()(
        pipette_motion_controller::PipetteMotionController<MEConfig>*
            controller,
        CanClient* can_client, UsageClient* usage_client,
        DriverClient* driver_client, MotionClient* motion_client) {
        auto handler = MotionControllerMessageHandler{
            *controller,    *can_client,    *usage_client,
            *driver_client, *motion_client, diag0_debounced};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

    void run_diag0_interrupt() {
        if (!diag0_debounced) {
            static_cast<void>(queue.try_write_isr(
                pipettes::task_messages::motor_control_task_messages::
                    RouteMotorDriverInterrupt{.message_index = 0,
                                              .debounce_count = 0}));
            diag0_debounced = true;
        }
    }

  private:
    QueueType& queue;
    std::atomic<bool> diag0_debounced = false;
};

}  // namespace motion_controller_task
}  // namespace tasks
}  // namespace pipettes
