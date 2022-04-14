#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "common/core/message_utils.hpp"
#include "motor-control/core/stepper_motor/tmc2130.hpp"
#include "motor-control/core/stepper_motor/tmc2130_driver.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "spi/core/messages.hpp"

namespace tmc2130 {

namespace tasks {

using SpiResponseMessage = std::tuple<spi::messages::TransactResponse>;
using CanMessageTuple = std::tuple<can_messages::ReadMotorDriverRegister,
                                    can_messages::SetupRequest,
                                    can_messages::WriteMotorDriverRegister,
                                    can_messages::WriteMotorCurrentRequest>;
using CanMessage =
    typename ::utils::TuplesToVariants<std::tuple<std::monostate>,
                                       CanMessageTuple>::type;
using TaskMessage = typename ::utils::VariantCat<
    std::variant<std::monostate>,
    typename ::utils::TuplesToVariants<CanMessageTuple,
                                       SpiResponseMessage>::type>::type;

/**
 * The handler of motor driver messages
 */
template <class Writer, message_writer_task::TaskClient CanClient,
          class TaskQueue>
class MotorDriverMessageHandler {
  public:
    MotorDriverMessageHandler(Writer& writer, CanClient& can_client,
                              TaskQueue& task_queue,
                              tmc2130::configs::TMC2130DriverConfig& configs)
        : driver(writer, task_queue, configs), can_client(can_client) {}
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
    void handle(const std::monostate m) { static_cast<void>(m); }

    void handle(const spi::messages::TransactResponse& m) {
        if (m.id.command_type ==
            static_cast<uint8_t>(spi::hardware::Mode::WRITE)) {
            driver.handle_spi_write(tmc2130::registers::Registers(m.id.token),
                                    m.rxBuffer);
        } else {
            auto data = driver.handle_spi_read(
                tmc2130::registers::Registers(m.id.token), m.rxBuffer);
            if (m.id.send_response) {
                can_messages::ReadMotorDriverRegisterResponse response_msg{
                    .reg_address = m.id.token,
                    .data = data,
                };
                can_client.send_can_message(can_ids::NodeId::host,
                                            response_msg);
            }
        }
    }

    void handle(const can_messages::SetupRequest& m) {
        LOG("Received motor setup request");
        driver.write_config();
    }

    void handle(const can_messages::WriteMotorDriverRegister& m) {
        LOG("Received write motor driver request: addr=%d, data=%d",
            m.reg_address, m.data);
        if (tmc2130::registers::is_valid_address(m.reg_address)) {
            driver.write(tmc2130::registers::Registers(m.reg_address), m.data);
        }
    }

    void handle(const can_messages::ReadMotorDriverRegister& m) {
        LOG("Received read motor driver request: addr=%d", m.reg_address);
        uint32_t data = 0;
        if (tmc2130::registers::is_valid_address(m.reg_address)) {
            driver.read(tmc2130::registers::Registers(m.reg_address), data);
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
        driver.set_current_control(driver.get_register_map().ihold_irun);
    }

    tmc2130::driver::TMC2130<Writer, TaskQueue> driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
<<<<<<< HEAD
template <template <class> class QueueImpl>
=======
template <template <class> class QueueImpl, class MotorDriverConfigs,
          message_writer_task::TaskClient CanClient, class SpiWriter>
>>>>>>> more edits to the motor controller driver
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class MotorDriverTask {
  public:
    using QueueType = QueueImpl<TaskMessage>;
<<<<<<< HEAD

    TMC2130MotorDriverTask(QueueType& queue) : queue{queue} {}
    TMC2130MotorDriverTask(const TMC2130MotorDriverTask& c) = delete;
    TMC2130MotorDriverTask(const TMC2130MotorDriverTask&& c) = delete;
    auto operator=(const TMC2130MotorDriverTask& c) = delete;
    auto operator=(const TMC2130MotorDriverTask&& c) = delete;
    ~TMC2130MotorDriverTask() = default;
=======
    MotorDriverTask(QueueType& queue) : queue{queue} {}
    MotorDriverTask(const MotorDriverTask& c) = delete;
    MotorDriverTask(const MotorDriverTask&& c) = delete;
    auto operator=(const MotorDriverTask& c) = delete;
    auto operator=(const MotorDriverTask&& c) = delete;
    ~MotorDriverTask() = default;
>>>>>>> more edits to the motor controller driver

    /**
     * Task entry point.
     */
<<<<<<< HEAD
    template <message_writer_task::TaskClient CanClient>
    [[noreturn]] void operator()(motor_driver::MotorDriver* driver,
                                 CanClient* can_client) {

        auto handler = MotorDriverMessageHandler{*driver, *can_client};
=======
    [[noreturn]] void operator()(MotorDriverConfigs* configs,
                                 CanClient* can_client, SpiWriter* writer) {
        auto handler = MotorDriverMessageHandler(*writer, *can_client,
                                                 get_queue(), *configs);
>>>>>>> more edits to the motor controller driver
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

}  // namespace tasks

}  // namespace tmc2130
