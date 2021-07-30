#pragma once

#include <array>

#include "can/core/can_bus.hpp"
#include "can/core/ids.hpp"
#include "can/core/parse.hpp"
#include "stm32g4xx_hal_fdcan.h"

using namespace can_bus;
using namespace can_ids;

/**
 * HAL FD CAN wrapper. Matches the CanBus concept.
 */
class HalCanBus {
  public:
    explicit HalCanBus(FDCAN_HandleTypeDef* handle);

    void add_filter(CanFilterType type, CanFilterConfig config, uint32_t val1,
                    uint32_t val2);

    void send(uint32_t arbitration_id, uint8_t* buffer,
              CanFDMessageLength buffer_length);

    static auto convert_length(uint32_t length) -> CanFDMessageLength;
  private:
    FDCAN_HandleTypeDef* handle;
    uint32_t filter_index = 0;
    FDCAN_TxHeaderTypeDef tx_header;

    static constexpr auto arbitration_id_type = FDCAN_STANDARD_ID;

    static auto convert_config(CanFilterConfig config) -> uint32_t;

    static auto convert_type(CanFilterType type) -> uint32_t;

    static auto convert_length(CanFDMessageLength length) -> uint32_t;

};
