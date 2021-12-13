#pragma once

#include "can/core/can_bus.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/message_writer.hpp"
#include "can/core/messages.hpp"

namespace device_info_handler {

using namespace can_ids;
using namespace can_message_writer;
using namespace can_messages;

/**
 * A HandlesMessages implementing class that will respond to DeviceInfoRequest
 * with a DeviceInfoResponse.
 *
 * @tparam Writer The can bus writer type.
 */
class DeviceInfoHandler {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param node_id The node id of this device
     * @param version The firmware version on this device
     */
    DeviceInfoHandler(MessageWriter2 &writer, uint32_t version)
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

    void visit(DeviceInfoRequest &m) { writer.write(NodeId::host, response); }

    MessageWriter2 &writer;
    DeviceInfoResponse response;
};

/**
 * Type short cut for creating dispatch parse target for the handler.
 */
using DispatchTarget =
    can_dispatch::DispatchParseTarget<DeviceInfoHandler, DeviceInfoRequest>;

}  // namespace device_info_handler
