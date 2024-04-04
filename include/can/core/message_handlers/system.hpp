#pragma once

#include <array>

#include "can/core/can_writer_task.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "common/core/app_update.h"

namespace can::message_handlers::system {

using namespace can::ids;
using namespace can::messages;

/**
 * A HandlesMessages implementing class that will respond to system messages.
 *
 * @tparam CanClient can writer task client
 */
template <can::message_writer_task::TaskClient CanClient>
class SystemMessageHandler {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param version The firmware version on this device
     */
    SystemMessageHandler(CanClient &writer, uint32_t version, uint32_t flags,
                         std::span<const char> version_sha,
                         char primary_revision, char secondary_revision, const char tertiary_revision[2],
                         uint8_t device_subid = 0)
        : writer(writer),
          response{.version = version,
                   .flags = flags,
                   .primary_revision = primary_revision,
                   .secondary_revision = secondary_revision,
                   .device_subidentifier = device_subid} {
        std::copy_n(version_sha.begin(),
                    std::min(version_sha.size(), response.shortsha.size()),
                    response.shortsha.begin());
        std::copy_n(&tertiary_revision[0], std::min(2 * sizeof(char), response.tertiary_revision.size()),
                    response.tertiary_revision.begin());
    }
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
    void visit(std::monostate &) {}

    void visit(DeviceInfoRequest &m) {
        can::messages::add_resp_ind(response, m);
        writer.send_can_message(can::ids::NodeId::host, response);
    }

    void visit(InitiateFirmwareUpdate &) { app_update_start(); }

    void visit(FirmwareUpdateStatusRequest &m) {
        auto status_response = FirmwareUpdateStatusResponse{
            .message_index = m.message_index, .flags = app_update_flags()};
        writer.send_can_message(can::ids::NodeId::host, status_response);
    }

    void visit(TaskInfoRequest &m) {
        auto tasks = std::array<TaskStatus_t, 20>{};
        auto num_tasks =
            uxTaskGetSystemState(tasks.data(), tasks.size(), nullptr);
        for (UBaseType_t i = 0; i < num_tasks; i++) {
            auto r = TaskInfoResponse{};
            can::messages::add_resp_ind(r, m);
            std::copy_n(tasks[i].pcTaskName, r.name.size(),  // NOLINT
                        r.name.begin());
            r.runtime_counter = tasks[i].ulRunTimeCounter;            // NOLINT
            r.stack_high_water_mark = tasks[i].usStackHighWaterMark;  // NOLINT
            r.state = tasks[i].eCurrentState;                         // NOLINT
            r.priority = tasks[i].uxCurrentPriority;                  // NOLINT
            writer.send_can_message(can::ids::NodeId::host, r);
        }
    }

    CanClient &writer;
    can::messages::DeviceInfoResponse response;
};

}  // namespace can::message_handlers::system
