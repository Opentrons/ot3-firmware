#pragma once

#include <array>

#include "can_bus.hpp"
#include "ids.hpp"
#include "parse.hpp"

namespace can_message_writer {

template <can_bus::CanBus Writer>
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
    template <can_parse::Serializable Serializable>
    void write(can_ids::NodeId node, const Serializable& message) {
        arbitration_id.parts.message_id = static_cast<unsigned int>(message.id);
        // TODO (al 2021-08-03): populate this from Message?
        arbitration_id.parts.function_code = 0;
        arbitration_id.parts.node_id = static_cast<unsigned int>(node);
        auto length = message.serialize(buffer.begin(), buffer.end());
        writer.send(arbitration_id.id, buffer.data(),
                    can_bus::to_canfd_length(length));
    }

  private:
    Writer& writer;
    std::array<uint8_t, 64> buffer;
    can_ids::ArbitrationId arbitration_id;
};

}  // namespace can_message_writer