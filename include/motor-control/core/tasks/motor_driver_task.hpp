#pragma once

#include <variant>

#include "can/core/messages.hpp"
#include "common/core/freertos_message_queue_poller.hpp"
#include "motor-control/core/motor_driver.hpp"
#include "motor-control/core/motor_driver_config.hpp"
#include "motor-control/core/tasks/messages.hpp"

namespace motor_driver_task {

using TaskMessage = motor_control_task_messages::MotorDriverTaskMessage;

/**
 * The handler of motor driver messages
 */
class MotorDriverMessageHandler {
  public:
    MotorDriverMessageHandler(motor_driver::MotorDriver& driver)
        : driver{driver} {}
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

    void handle(const can_messages::SetupRequest& m) { driver.setup(); }

    void handle(const can_messages::WriteMotorDriverRegister& m) {
        if (motor_driver_config::DriverRegisters::is_valid_address(
                m.reg_address)) {
            driver.write(
                motor_driver_config::DriverRegisters::Addresses(m.reg_address),
                m.data);
        }
    }

    void handle(const can_messages::ReadMotorDriverRegister& m) {
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
        //        message_writer.write(NodeId::host, response_msg);
    }

    motor_driver::MotorDriver& driver;
};

/**
 * The task type.
 */
using MotorDriverTask =
    freertos_message_queue_poller::FreeRTOSMessageQueuePoller<
        TaskMessage, MotorDriverMessageHandler>;

/**
 * Concept describing a class that can message this task.
 * @tparam TaskHolder
 */
template <typename TaskHolder>
concept TaskClient = requires(TaskHolder holder, const TaskMessage& m) {
    {holder.send_motor_driver_queue(m)};
};

}  // namespace motor_driver_task