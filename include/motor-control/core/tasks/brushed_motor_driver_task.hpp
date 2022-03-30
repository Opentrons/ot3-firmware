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
template <message_writer_task::TaskClient CanClient>
class MotorDriverMessageHandler {
  public:
    MotorDriverMessageHandler(
        brushed_motor_driver::BrushedMotorDriverIface& driver,
        CanClient& can_client)
        : driver{driver}, can_client{can_client} {}
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

    void handle(const can_messages::SetupRequest& m) {
        LOG("Received motor setup request");
        driver.setup();
    }

    void handle(const can_messages::SetBrushedMotorVrefRequest& m) {
        auto val = fixed_point_to_float(m.v_ref, 15);
        LOG("Received set motor reference voltage request,  vref=%f", val);
        driver.set_reference_voltage(val);
    }

    void handle(const can_messages::SetBrushedMotorPwmRequest& m) {
        LOG("Received set motor PWM request, freq=%d, duty_cycle=%d", m.freq,
            m.duty_cycle);
        driver.update_pwm_settings(m.freq, m.duty_cycle);
    }

    brushed_motor_driver::BrushedMotorDriverIface& driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl,
          message_writer_task::TaskClient CanClient>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MotorDriverTask {
  public:
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
    [[noreturn]] void operator()(
        brushed_motor_driver::BrushedMotorDriverIface* driver,
        CanClient* can_client) {
        // Set up the motor driver.
        //        driver->setup();

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