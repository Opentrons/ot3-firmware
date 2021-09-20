#pragma once

#include <array>

#include "arbitration_id.hpp"
#include "can_bus.hpp"
#include "message_core.hpp"

namespace can_message_writer {

template <can_bus::CanBusWriter Writer>
class MessageWriter {
  public:
    explicit MessageWriter(Writer& writer) : writer{writer} {}

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <message_core::Serializable Serializable>
    void write(can_ids::NodeId node, const Serializable& message) {
        auto arbitration_id = can_arbitration_id::ArbitrationId{.id = 0};
        arbitration_id.parts.message_id = static_cast<uint16_t>(message.id);
        // TODO (al 2021-08-03): populate this from Message?
        arbitration_id.parts.function_code = 0;
        arbitration_id.parts.node_id = static_cast<unsigned int>(node);
        auto length = message.serialize(buffer.begin(), buffer.end());
        writer.send(arbitration_id.id, buffer.data(),
                    can_bus::to_canfd_length(length));
    }

  private:
    Writer& writer;
    std::array<uint8_t, message_core::MaxMessageSize> buffer{};
};

}  // namespace can_message_writer