#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/stepper_motor/motor_driver.hpp"
#include "motor-control/core/stepper_motor/motor_driver_config.hpp"
#include "motor-control/core/stepper_motor/tmc2130_registers.hpp"
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

    void handle(const can_messages::WriteMotorDriverRegister& m) {
        LOG("Received write motor driver request: addr=%d, data=%d",
            m.reg_address, m.data);
        if (motor_driver_config::DriverRegisters::is_valid_address(
                m.reg_address)) {
            driver.write(tmc2130::Registers(m.reg_address), m.data);
        }
    }

    void handle(const can_messages::ReadMotorDriverRegister& m) {
        LOG("Received read motor driver request: addr=%d", m.reg_address);
        uint32_t data = 0;
        if (motor_driver_config::DriverRegisters::is_valid_address(
                m.reg_address)) {
            data = driver.read(tmc2130::Registers(m.reg_address), data);
        }
        can_messages::ReadMotorDriverRegisterResponse response_msg{
            .reg_address = m.reg_address,
            .data = data,
        };
        can_client.send_can_message(can_ids::NodeId::host, response_msg);
    }

    void handle(const can_messages::WriteMotorCurrentRequest& m) {
        LOG("Received write motor current request: hold_current=%d, "
            "run_current=%d",
            m.hold_current, m.run_current);

        if (m.hold_current != 0U) {
            driver.tmc2130.get_register_map().ihold_irun.hold_current =
                driver.tmc2130.convert_to_tmc2130_current_value(m.hold_current);
        };
        if (m.run_current != 0U) {
            driver.tmc2130.get_register_map().ihold_irun.run_current =
                driver.tmc2130.convert_to_tmc2130_current_value(m.run_current);
        }
        driver.tmc2130.write_config();
    }

    motor_driver::MotorDriver& driver;
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
    [[noreturn]] void operator()(motor_driver::MotorDriver* driver,
                                 CanClient* can_client) {
        // Set up the motor driver.
        driver->setup();

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
    {client.send_motor_driver_queue(m)};
};

}  // namespace motor_driver_task