#pragma once

#include <variant>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/logging.hpp"
#include "head/core/adc.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/core/tool_list.hpp"

using namespace ot3_tool_list;
namespace presence_sensing_driver_task {

using TaskMessage =
    std::variant<std::monostate,
                 can_messages::ReadPresenceSensingVoltageRequest,
                 can_messages::AttachedToolsRequest>;

/**
 * The handler of Presence Sensing messages
 */
template <message_writer_task::TaskClient CanClient>
class PresenceSensingDriverMessageHandler {
  public:
    PresenceSensingDriverMessageHandler(
        presence_sensing_driver::PresenceSensingDriver& driver,
        CanClient& can_client)
        : driver{driver}, can_client{can_client} {}
    PresenceSensingDriverMessageHandler(
        const PresenceSensingDriverMessageHandler& c) = delete;
    PresenceSensingDriverMessageHandler(
        const PresenceSensingDriverMessageHandler&& c) = delete;
    auto operator=(const PresenceSensingDriverMessageHandler& c) = delete;
    auto operator=(const PresenceSensingDriverMessageHandler&& c) = delete;
    ~PresenceSensingDriverMessageHandler() = default;

    /**
     * Called upon arrival of new message
     * @param message
     */
    void handle_message(const TaskMessage& message) {
        std::visit([this](auto m) { this->visit(m); }, message);
    }

  private:
    void visit(std::monostate& m) {}

    void visit(can_messages::ReadPresenceSensingVoltageRequest& m) {
        auto voltage_read = driver.get_readings();

        can_messages::ReadPresenceSensingVoltageResponse resp{
            .z_motor = voltage_read.z_motor,
            .a_motor = voltage_read.a_motor,
            .gripper = voltage_read.gripper,
        };
        can_client.send_can_message(can_ids::NodeId::host, resp);
    }

    void visit(can_messages::AttachedToolsRequest& m) {
        auto tools = AttachedTool(driver.get_readings(), get_tool_list());
        if (tools.z_motor != driver.get_current_tools().z_motor ||
            tools.a_motor != driver.get_current_tools().a_motor ||
            tools.gripper != driver.get_current_tools().gripper) {
            can_messages::PushToolsDetectedNotification resp{
                .z_motor = (tools.z_motor),
                .a_motor = (tools.a_motor),
                .gripper = (tools.gripper),
            };
            driver.set_current_tools(tools);
            can_client.send_can_message(can_ids::NodeId::host, resp);
        }
    }

    presence_sensing_driver::PresenceSensingDriver& driver;
    CanClient& can_client;
};

/**
 * The task type.
 */
template <template <class> class QueueImpl,
          message_writer_task::TaskClient CanClient>
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
    [[noreturn]] void operator()(
        presence_sensing_driver::PresenceSensingDriver* driver,
        CanClient* can_client) {
        auto handler =
            PresenceSensingDriverMessageHandler{*driver, *can_client};
        TaskMessage message{};

        for (;;) {
            if (queue.try_read(&message, queue.max_delay)) {
                handler.handle_message(message);
            }
        }
    }

    [[nodiscard]] auto get_queue() const -> QueueType& { return queue; }

    void notifier_callback() {
        this->queue.try_write(
            TaskMessage(can_messages::AttachedToolsRequest()));
    }

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