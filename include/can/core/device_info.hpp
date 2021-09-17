#pragma once

#include "can_bus.hpp"
#include "ids.hpp"
#include "message_writer.hpp"
#include "messages.hpp"

using namespace can_ids;
using namespace can_message_writer;
using namespace can_messages;

namespace can_device_info {

/**
 * A HandlesMessages implementing class that will respond to DeviceInfoRequest
 * with a DeviceInfoResponse.
 *
 * @tparam Writer The can bus writer type.
 */
template <can_bus::CanBusWriter Writer>
class DeviceInfoHandler {
  public:
    /**
     * Constructor
     *
     * @param writer A message writer for sending the response
     * @param node_id The node id of this device
     * @param version The firmware version on this device
     */
    DeviceInfoHandler(MessageWriter<Writer> &writer, NodeId node_id,
                      uint32_t version)
        : writer(writer), response{node_id, version} {}
    DeviceInfoHandler(const DeviceInfoHandler &) = delete;
    DeviceInfoHandler(const DeviceInfoHandler &&) = delete;
    DeviceInfoHandler &operator=(const DeviceInfoHandler &) = delete;
    DeviceInfoHandler &&operator=(const DeviceInfoHandler &&) = delete;

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

    MessageWriter<Writer> &writer;
    DeviceInfoResponse response;
};

}  // namespace can_device_info