#pragma once

#include "can/core/can_writer_task.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"

namespace device_info_handler {

using namespace can_ids;
using namespace can_messages;

/**
 * A HandlesMessages implementing class that will respond to DeviceInfoRequest
 * with a DeviceInfoResponse.
 *
 * @tparam CanClient can writer task cliene
 */
template <message_writer_task::TaskClient CanClient>
class DeviceInfoHandler {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param version The firmware version on this device
     */
    DeviceInfoHandler(CanClient &writer, uint32_t version)
        : writer(writer), response{.version = version} {}
    DeviceInfoHandler(const DeviceInfoHandler &) = delete;
    DeviceInfoHandler(const DeviceInfoHandler &&) = delete;
    auto operator=(const DeviceInfoHandler &) -> DeviceInfoHandler & = delete;
    auto operator=(const DeviceInfoHandler &&) -> DeviceInfoHandler && = delete;
    ~DeviceInfoHandler() = default;

    using MessageType = std::variant<std::monostate, DeviceInfoRequest>;

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

    CanClient &writer;
    can_messages::DeviceInfoResponse response;
};

/**
 * Type short cut for creating dispatch parse target for the handler.
 */
template <message_writer_task::TaskClient CanClient>
using DispatchTarget =
    can_dispatch::DispatchParseTarget<DeviceInfoHandler<CanClient>,
                                      DeviceInfoRequest>;

}  // namespace device_info_handler
