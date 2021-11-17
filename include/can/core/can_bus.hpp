#pragma once

#include <concepts>
#include <cstdint>
#include <functional>

#include "arbitration_id.hpp"
#include "ids.hpp"
#include "parse.hpp"
#include "types.h"

namespace can_bus {

using namespace can_ids;


class CanBus {
  public:
    using IncomingMessageCallback = std::function<void(uint32_t arbitration_id, uint8_t* data, uint8_t length)>;

    /**
     * Set callback for incoming messages.
     *
     * @param callback
     */
    virtual void set_incoming_message_callback(IncomingMessageCallback callback) = 0;

    virtual ~CanBus() {}
};


/**
 * Can bus writer abstract base class.
 */
class CanBusWriter {
  public:
    /**
     * Write method.
     *
     * @param arbitration_id A 32-bit arbitration id
     * @param buffer The data buffer.
     * @param buffer_length The buffer length.
     */
    virtual void send(uint32_t arbitration_id, uint8_t* buffer,
                      CanFDMessageLength buffer_length) = 0;

    virtual ~CanBusWriter() {}
};

/**
 * Can incoming message filter setup.
 */
class CanBusFilters {
  public:
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


    virtual ~CanBusFilters() {}
};

/**
 * Set up the can bus receive filter to receive only broadcast messages and
 * messages targeting node_id.
 *
 * @param can_filters CanBusFilters interface.
 * @param node_id The node id to allow.
 */
void setup_node_id_filter(CanBusFilters& can_filters, NodeId node_id);

}  // namespace can_bus