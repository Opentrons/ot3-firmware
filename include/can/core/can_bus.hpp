#pragma once

#include <concepts>
#include <cstdint>

#include "arbitration_id.hpp"
#include "can/core/ids.hpp"
#include "parse.hpp"
#include "types.h"

namespace can::bus {

using namespace can::ids;

/**
 * Abstract base class of a CAN bus.
 */
// NOLINTNEXTLINE(cppcoreguidelines-special-member-functions)
class CanBus {
  public:
    /**
     * Incoming message callback method
     */
    using IncomingMessageCallback = void (*)(void* cb_data, uint32_t identifier,
                                             uint8_t* data, uint8_t length);

    /**
     * Set callback for incoming messages.
     *
     * @param cb_data data that will be passed back to the callback.
     * @param callback callback function reporting new CAN message.
     */
    virtual void set_incoming_message_callback(
        void* cb_data, IncomingMessageCallback callback) = 0;

    /**
     * Write method.
     *
     * @param arbitration_id A 32-bit arbitration id
     * @param buffer The data buffer.
     * @param buffer_length The buffer length.
     */
    virtual void send(uint32_t arbitration_id, uint8_t* buffer,
                      CanFDMessageLength buffer_length) = 0;

    /**
     * Create a filter for incoming messages.
     *
     * @param type the type of filter
     * @param config the filter configuration
     * @param val1 depends on the type. Is either a filter, exact arbitration
     * id, or minimum arbitration id
     * @param val2 depends on the type. Is either a mask, exact arbitration id,
     * or maximum arbitration id
     */
    virtual void add_filter(CanFilterType type, CanFilterConfig config,
                            uint32_t val1, uint32_t val2) = 0;

    /**
     * Set up the can bus receive filter to receive only broadcast messages and
     * messages targeting node_id.
     *
     * @param node_id The node id to allow.
     */
    void setup_node_id_filter(NodeId node_id);

    virtual ~CanBus() = default;
};

}  // namespace can_bus
