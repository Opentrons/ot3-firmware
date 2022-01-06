#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.hpp"
#include "presence-sensing/core/presence_sensing_driver.hpp"
#include "presence-sensing/core/presence_sensing_driver_config.hpp"
#include "presence-sensing/core/tasks/messages.hpp"
#include "common/core/adc.hpp"

namespace presence_sensing_driver_task {

using TaskMessage = presence_sensing_task_messages::PresenceSensingTaskMessage;

/**
 * The handler of Presence Sensing messages
 */
template <message_writer_task::TaskClient CanClient,
         adc::has_get_reading ADCDriver>
class PresenceSensingDriverMessageHandler {
  public:
    PresenceSensingDriverMessageHandler(presence_sensing_driver::PresenceSensingDriver<ADCDriver>& driver,
                              CanClient& can_client)
        : driver{driver}, can_client{can_client} {}
    PresenceSensingDriverMessageHandler(const PresenceSensingDriverMessageHandler& c) = delete;
    PresenceSensingDriverMessageHandler(const PresenceSensingDriverMessageHandler&& c) = delete;
    auto operator=(const PresenceSensingDriverMessageHandler& c) = delete;
    auto operator=(const PresenceSensingDriverMessageHandler&& c) = delete;
    ~PresenceSensingDriverMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->handle(m); }, message);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(presence_sensing_messages::GetVoltage &m) {
        auto voltage_read = driver.get_readings();

        GetPresenceSensingResponse response_msg{
            .z_motor = voltage_read.z_motor,
            .a_motor = voltage_read.a_motor,
            .gripper = voltage_read.gripper,
        };
        can_client.send_can_message(NodeId::host, response_msg);
    }
    

    presence_sensing_driver::PresenceSensingDriver<ADCDriver>& driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl,
          message_writer_task::TaskClient CanClient,
          adc::has_get_reading ADCDriver>
requires MessageQueue<QueueImpl<TaskMessage>, TaskMessage>
class PresenceSensingDriverTask {
  public:
    using QueueType = QueueImpl<TaskMessage>;
    PresenceSensingDriverTask(QueueType& queue) : queue{queue} {}
    PresenceSensingDriverTask(const PresenceSensingDriverTask& c) = delete;
    PresenceSensingDriverTask(const PresenceSensingDriverTask&& c) = delete;
    auto operator=(const PresenceSensingDriverTask& c) = delete;
    auto operator=(const PresenceSensingDriverTask&& c) = delete;
    ~PresenceSensingDriverTask() = default;

    /**
     * Task entry point.
     */
    [[noreturn]] void operator()(presence_sensing_driver::PresenceSensingDriver<ADCDriver>* driver,
                                 CanClient* can_client) {
        auto handler = PresenceSensingDriverMessageHandler{*driver, *can_client};
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
    {client.send_presence_sensing_driver_queue(m)};
};

}  // namespace presence_sensing_driver_task