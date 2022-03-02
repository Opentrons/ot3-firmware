#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/app_update.h"

namespace system_handler {

using namespace can_ids;
using namespace can_messages;

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 */
template <message_writer_task::TaskClient CanClient>
class SystemMessageHandler {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param version The firmware version on this device
     */
    SystemMessageHandler(CanClient &writer, uint32_t version)
        : writer(writer), response{.version = version} {}
    SystemMessageHandler(const SystemMessageHandler &) = delete;
    SystemMessageHandler(const SystemMessageHandler &&) = delete;
    auto operator=(const SystemMessageHandler &)
        -> SystemMessageHandler & = delete;
    auto operator=(const SystemMessageHandler &&)
        -> SystemMessageHandler && = delete;
    ~SystemMessageHandler() = default;

    using MessageType =
        std::variant<std::monostate, DeviceInfoRequest, InitiateFirmwareUpdate,
                     FirmwareUpdateStatusRequest, TaskInfoRequest>;

    /**
     * Message handler
     * @param m The incoming message.
     */
    void handle(MessageType &m) {
        std::visit([this](auto o) { this->visit(o); }, m);
    }

  private:
    void visit(std::monostate &m) {}

    void visit(DeviceInfoRequest &m) {
        writer.send_can_message(can_ids::NodeId::host, response);
    }

    void visit(InitiateFirmwareUpdate &m) { app_update_start(); }

    void visit(FirmwareUpdateStatusRequest &m) {
        auto status_response =
            FirmwareUpdateStatusResponse{.flags = app_update_flags()};
        writer.send_can_message(can_ids::NodeId::host, status_response);
    }

    void visit(TaskInfoRequest &m) {
        auto tasks = std::array<TaskStatus_t, 20>{};
        auto num_tasks =
            uxTaskGetSystemState(tasks.data(), tasks.size(), nullptr);
        for (UBaseType_t i = 0; i < num_tasks; i++) {
            auto r = TaskInfoResponse{};
            std::copy_n(tasks[i].pcTaskName, r.name.size(), r.name.begin());
            r.runtime_counter = tasks[i].ulRunTimeCounter;
            r.stack_high_water_mark = tasks[i].usStackHighWaterMark;
            r.state = tasks[i].eCurrentState;
            r.priority = tasks[i].uxCurrentPriority;
            writer.send_can_message(can_ids::NodeId::host, r);
        }
    }

    CanClient &writer;
    can_messages::DeviceInfoResponse response;
};

}  // namespace system_handler
