#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "motor-control/core/stepper_motor/motor_driver.hpp"
#include "motor-control/core/stepper_motor/motor_driver_config.hpp"
#include "motor-control/core/stepper_motor/tmc2130_config.hpp"
#include "motor-control/core/stepper_motor/tmc2130_registers.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "spi/core/messages.hpp"

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
        : driver{configs, writer}, can_client{can_client} {

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

    void handle(const spi::messages::TransactResponse& m) {
        auto data = driver.handle_spi_response(m.success, m.id, m.rxBuffer);
        if (m.send_to_can) {
            can_messages::ReadMotorDriverRegisterResponse response_msg{
                .reg_address = m.reg_address,
                .data = data,
            };
            can_client.send_can_message(can_ids::NodeId::host, response_msg);
        }
    }

    void handle(const can_messages::SetupRequest& m) {
        LOG("Received motor setup request");
        driver.write_config();
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
            driver.read(tmc2130::Registers(m.reg_address), data);
        }
    }

    void handle(const can_messages::WriteMotorCurrentRequest& m) {
        LOG("Received write motor current request: hold_current=%d, "
            "run_current=%d",
            m.hold_current, m.run_current);

        if (m.hold_current != 0U) {
            driver.get_register_map().ihold_irun.hold_current =
                driver.convert_to_tmc2130_current_value(m.hold_current);
        };
        if (m.run_current != 0U) {
            driver.get_register_map().ihold_irun.run_current =
                driver.convert_to_tmc2130_current_value(m.run_current);
        }
        driver.write(tmc2130::Registers::IHOLD_IRUN, driver.get_current_settings());
    }

    tmc2130::TMC2130& driver;
    SpiTransactManager& _spi_manager;
    tmc2130::TMC2130DriverConfig configurations;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class TMC2130MotorDriverTask {
  public:
    using QueueType = QueueImpl<TaskMessage>;

    TMC2130MotorDriverTask(QueueType& queue) : queue{queue} {}
    TMC2130MotorDriverTask(const TMC2130MotorDriverTask& c) = delete;
    TMC2130MotorDriverTask(const TMC2130MotorDriverTask&& c) = delete;
    auto operator=(const TMC2130MotorDriverTask& c) = delete;
    auto operator=(const TMC2130MotorDriverTask&& c) = delete;
    ~TMC2130MotorDriverTask() = default;

    /**
     * Task entry point.
     */
    template <message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(motor_driver::MotorDriver* driver,
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
    {client.send_motor_driver_queue(m)};
};

}  // namespace motor_driver_task