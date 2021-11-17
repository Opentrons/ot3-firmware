#pragma once

#include <array>

#include "arbitration_id.hpp"
#include "can_bus.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "common/core/synchronization.hpp"
#include "ids.hpp"
#include "message_core.hpp"

namespace can_message_writer {

class MessageWriter {
  public:
    explicit MessageWriter(can_bus::CanBusWriter& writer,
                           can_ids::NodeId node_id)
        : writer{writer}, node_id(node_id) {}

    /**
     * Write a message to the can bus
     *
     * @tparam Serializable The message type
     * @param node The node id
     * @param message The message to send
     */
    template <message_core::CanResponseMessage ResponseMessage>
    void write(can_ids::NodeId node, ResponseMessage& message) {
        auto lock = synchronization::Lock{mutex};
        arbitration_id.id = 0;
        arbitration_id.parts.message_id = static_cast<uint16_t>(message.id);
        // TODO (al 2021-08-03): populate this from Message?
        arbitration_id.parts.function_code = 0;
        arbitration_id.parts.node_id = static_cast<unsigned int>(node);
        message.set_node_id(node_id);
        auto length = message.serialize(buffer.begin(), buffer.end());
        writer.send(arbitration_id.id, buffer.data(),
                    can_bus::to_canfd_length(length));
    }

  private:
    can_bus::CanBusWriter& writer;
    freertos_synchronization::FreeRTOSMutex mutex{};
    can_arbitration_id::ArbitrationId arbitration_id{};
    std::array<uint8_t, message_core::MaxMessageSize> buffer{};
    can_ids::NodeId node_id;
};

}  // namespace can_message_writer