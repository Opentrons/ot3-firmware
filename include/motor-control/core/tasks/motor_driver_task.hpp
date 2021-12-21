#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.hpp"
#include "motor-control/core/motor_driver.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace motor_driver_task {

using TaskMessage = motor_control_task_messages::MotorDriverTaskMessage;

/**
 * The handler of motor driver messages
 */
template <message_writer_task::TaskClient CanClient>
class MotorDriverMessageHandler {
  public:
    MotorDriverMessageHandler(motor_driver::MotorDriver& driver,
                              CanClient& can_client)
        : driver{driver}, can_client{can_client} {}
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
        LOG("Received motor setup request\n");
        driver.setup();
    }

    void handle(const can_messages::WriteMotorDriverRegister& m) {
        LOG("Received write motor driver request: addr=%d, data=%d\n",
            m.reg_address, m.data);
        if (motor_driver_config::DriverRegisters::is_valid_address(
                m.reg_address)) {
            driver.write(
                motor_driver_config::DriverRegisters::Addresses(m.reg_address),
                m.data);
        }
    }

    void handle(const can_messages::ReadMotorDriverRegister& m) {
        LOG("Received read motor driver request: addr=%d\n", m.reg_address);
        uint32_t data = 0;
        if (motor_driver_config::DriverRegisters::is_valid_address(
                m.reg_address)) {
            driver.read(
                motor_driver_config::DriverRegisters::Addresses(m.reg_address),
                data);
        }
        can_messages::ReadMotorDriverRegisterResponse response_msg{
            .reg_address = m.reg_address,
            .data = data,
        };
        can_client.send_can_message(can_ids::NodeId::host, response_msg);
    }

    motor_driver::MotorDriver& driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <message_writer_task::TaskClient CanClient>
class MotorDriverTask {
  public:
    using QueueType = freertos_message_queue::FreeRTOSMessageQueue<TaskMessage>;
    MotorDriverTask(QueueType& queue) : queue{queue} {}
    ~MotorDriverTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(motor_driver::MotorDriver* driver,
                                 CanClient* can_client) {
        auto handler = MotorDriverMessageHandler{*driver, *can_client};
        TaskMessage message{};
        for (;;) {
            if (queue.try_read(&message, portMAX_DELAY)) {
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
    {client.send_motor_driver_queue(m)};
};

}  // namespace motor_driver_task