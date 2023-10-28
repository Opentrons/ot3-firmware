#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.h"
#include "common/core/message_utils.hpp"
#include "motor-control/core/stepper_motor/tmc2160.hpp"
#include "motor-control/core/stepper_motor/tmc2160_driver.hpp"
#include "motor-control/core/tasks/messages.hpp"
#include "motor-control/core/tasks/tmc_motor_driver_common.hpp"
#include "spi/core/messages.hpp"

namespace tmc2160 {

namespace tasks {

namespace gear {

using TaskMessage = tmc::tasks::GearTaskMessage;
/**
 * The handler of motor driver messages
 */
template <class Writer, can::message_writer_task::TaskClient CanClient,
          class TaskQueue>
class MotorDriverMessageHandler {
  public:
    MotorDriverMessageHandler(Writer& writer, CanClient& can_client,
                              TaskQueue& task_queue,
                              tmc2160::configs::TMC2160DriverConfig& configs)
        : driver(writer, task_queue, configs), can_client(can_client) {
        driver.write_config();
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
    void handle(const std::monostate m) { static_cast<void>(m); }

    void handle(const spi::messages::TransactResponse& m) {
        if (m.id.command_type ==
                static_cast<uint8_t>(spi::hardware::Mode::WRITE) &&
            !m.success) {
            driver.handle_spi_write_failure(tmc2160::registers::Registers(
                static_cast<uint8_t>(m.id.token)));
        } else if (m.id.command_type ==
                   static_cast<uint8_t>(spi::hardware::Mode::READ)) {
            auto data = driver.handle_spi_read(
                tmc2160::registers::Registers(static_cast<uint8_t>(m.id.token)),
                m.rxBuffer);
            can::messages::ReadMotorDriverRegisterResponse response_msg{
                .message_index = m.id.message_index,
                .reg_address = static_cast<uint8_t>(m.id.token),
                .data = data,
            };
            can_client.send_can_message(can::ids::NodeId::host, response_msg);
        }
    }

    void handle(const can::messages::GearWriteMotorDriverRegister& m) {
        LOG("Received write motor driver request: addr=%d, data=%d",
            m.reg_address, m.data);
        if (tmc2160::registers::is_valid_address(m.reg_address)) {
            driver.write(tmc2160::registers::Registers(m.reg_address), m.data);
        }
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    void handle(const can::messages::GearReadMotorDriverRegister& m) {
        LOG("Received read motor driver request: addr=%d", m.reg_address);
        uint32_t data = 0;
        if (tmc2160::registers::is_valid_address(m.reg_address)) {
            driver.read(tmc2160::registers::Registers(m.reg_address), data, m.message_index);
        }
    }

    void handle(const can::messages::ReadMotorDriverErrorStatus& m) {
        LOG("Received read motor driver error register request");
        uint32_t data = 0;
        std::array tags{spi::utils::ResponseTag::IS_ERROR_RESPONSE};
        uint8_t tag_byte = spi::utils::byte_from_tags(tags);
        driver.read(tmc2160::registers::Registers::DRVSTATUS, data, m.message_index, tag_byte);
    }

    void handle(const can::messages::GearWriteMotorCurrentRequest& m) {
        LOG("Received gear write motor current request: hold_current=%d, "
            "run_current=%d",
            m.hold_current, m.run_current);
        if (m.hold_current != 0U) {
            driver.get_register_map().ihold_irun.hold_current =
                driver.convert_to_tmc2160_current_value(m.hold_current);
        };
        if (m.run_current != 0U) {
            driver.get_register_map().ihold_irun.run_current =
                driver.convert_to_tmc2160_current_value(m.run_current);
        }
        driver.set_current_control(driver.get_register_map().ihold_irun);
        can_client.send_can_message(can::ids::NodeId::host,
                                    can::messages::ack_from_request(m));
    }

    tmc2160::driver::TMC2160<Writer, TaskQueue> driver;
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

    template <can::message_writer_task::TaskClient CanClient,
              class MotorDriverConfigs, class SpiWriter>
    [[noreturn]] void operator()(MotorDriverConfigs* configs,
                                 CanClient* can_client, SpiWriter* writer) {
        auto handler = MotorDriverMessageHandler(*writer, *can_client,
                                                 get_queue(), *configs);
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

}  // namespace gear

}  // namespace tasks

}  // namespace tmc2160
