#pragma once


#include "can/firmware/hal_can.h"
#include "can/core/can_bus.hpp"

namespace hal_can_bus {

using namespace can_bus;
using namespace can_ids;

/**
 * HAL FD CAN wrapper.
 */
class HalCanBus : public CanBus{
  public:
    /**
     * Construct
     * @param handle A pointer to an initialized FDCAN_HandleTypeDef
     */
    explicit HalCanBus(HAL_CAN_HANDLE handle): handle{handle} {}

    HalCanBus(const HalCanBus&) = delete;
    HalCanBus& operator=(const HalCanBus&) = delete;
    HalCanBus(const HalCanBus&&) = delete;
    HalCanBus&& operator=(const HalCanBus&&) = delete;

    /**
     * Set the incoming message callback.
     * @param callback
     */
    void set_incoming_message_callback(IncomingMessageCallback callback);

    /**
     * Add an arbitration id filter.
     *
     * @param type the type of filter
     * @param config the filter configuration
     * @param val1 depends on the type. Is either a filter, exact arbitration
     * id, or minimum arbitration id
     * @param val2 depends on the type. Is either a mask, exact arbitration id,
     * or maximum arbitration id
     */
    void add_filter(CanFilterType type, CanFilterConfig config, uint32_t val1,
                    uint32_t val2);

    /**
     * Send a buffer on can bus
     * @param arbitration_id The arbitration id
     * @param buffer buffer to send
     * @param buffer_length length of buffer
     */
    void send(uint32_t arbitration_id, uint8_t* buffer,
              CanFDMessageLength buffer_length);

  private:
    HAL_CAN_HANDLE handle;
    uint32_t filter_index = 0;
};

}  // namespace hal_can_bus
