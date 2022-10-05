#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/brushed_motor/driver_interface.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/utils.hpp"

namespace brushed_motor_driver_task {

using TaskMessage = motor_control_task_messages::BrushedMotorDriverTaskMessage;

/**
 * The handler of brushed motor driver messages
 */
template <can::message_writer_task::TaskClient CanClient>
class MotorDriverMessageHandler {
  public:
    MotorDriverMessageHandler(
        brushed_motor_driver::BrushedMotorDriverIface& driver,
        CanClient& can_client)
        : driver{driver}, can_client{can_client} {
        driver.setup();
    }
    MotorDriverMessageHandler(const MotorDriverMessageHandler& c) = delete;
    MotorDriverMessageHandler(const MotorDriverMessageHandler&& c) = delete;
    auto operator=(const MotorDriverMessageHandler& c) = delete;
    auto operator=(const MotorDriverMessageHandler&& c) = delete;
    ~MotorDriverMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void handle(std::monostate m) { static_cast<void>(m); }

    void handle(const can::messages::SetBrushedMotorVrefRequest& m) {
        auto val = fixed_point_to_float(m.v_ref, 16);
        LOG("Received set motor reference voltage request, vref=%f", val);
        driver.set_reference_voltage(val);
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::Acknowledgment{.message_index = m.message_index});
    }

    void handle(const can::messages::SetBrushedMotorPwmRequest& m) {
        LOG("Received set motor PWM request, duty_cycle=%d", m.duty_cycle);
        driver.update_pwm_settings(m.duty_cycle);
        can_client.send_can_message(
            can::ids::NodeId::host,
            can::messages::Acknowledgment{.message_index = m.message_index});
    }

    brushed_motor_driver::BrushedMotorDriverIface& driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MotorDriverTask {
  public:
    using Messages = TaskMessage;
    using QueueType = QueueImpl<TaskMessage>;
    MotorDriverTask(QueueType& queue) : queue{queue} {}
    MotorDriverTask(const MotorDriverTask& c) = delete;
    MotorDriverTask(const MotorDriverTask&& c) = delete;
    auto operator=(const MotorDriverTask& c) = delete;
    auto operator=(const MotorDriverTask&& c) = delete;
    ~MotorDriverTask() = default;

    /**
     * Task entry point.
     */
    template <can::message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(
        brushed_motor_driver::BrushedMotorDriverIface* driver,
        CanClient* can_client) {
        auto handler = MotorDriverMessageHandler{*driver, *can_client};
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
    {client.send_brushed_motor_driver_queue(m)};
};

}  // namespace brushed_motor_driver_task
