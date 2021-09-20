#pragma once

#include <array>

#include "can/core/can_bus.hpp"
#include "can/core/ids.hpp"
#include "can/core/parse.hpp"
#include "common/core/freertos_synchronization.hpp"
#include "stm32g4xx_hal_fdcan.h"

using namespace can_bus;
using namespace can_ids;

namespace hal_can_bus {

/**
 * HAL FD CAN wrapper. Matches the CanBus concept.
 */
class HalCanBus {
  public:
    /**
     * Construct
     * @param handle A pointer to an initialized FDCAN_HandleTypeDef
     */
    explicit HalCanBus(FDCAN_HandleTypeDef* handle);

    HalCanBus(const HalCanBus&) = delete;
    HalCanBus& operator=(const HalCanBus&) = delete;
    HalCanBus(const HalCanBus&&) = delete;
    HalCanBus&& operator=(const HalCanBus&&) = delete;

    /**
     * Start the can bus.
     * @return HAL_OK if all went well
     */
    auto start() -> HAL_StatusTypeDef;

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
    FDCAN_HandleTypeDef* handle;
    uint32_t filter_index = 0;
    FDCAN_TxHeaderTypeDef tx_header;
    freertos_synchronization::FreeRTOSMutex mutex{};

    static constexpr auto arbitration_id_type = FDCAN_EXTENDED_ID;
};

}