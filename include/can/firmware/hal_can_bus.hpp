#pragma once

#include <array>

#include "can/core/can_bus.hpp"
#include "can/core/ids.hpp"
#include "can/core/parse.hpp"
#include "stm32g4xx_hal_fdcan.h"

using namespace can_bus;
using namespace can_ids;

class HalCanBus {
  public:
    explicit HalCanBus(FDCAN_HandleTypeDef* handle);

    void add_filter(CanFilterType type, CanFilterConfig config, uint32_t val1,
                    uint32_t val2);

    template <can_parse::Serializable Message>
    void send(uint32_t arbitration_id, const Message& message);

  private:
    FDCAN_HandleTypeDef* handle;
    uint32_t filter_index = 0;
    FDCAN_TxHeaderTypeDef tx_header;
    std::array<uint8_t, 64> tx_buffer;

    static constexpr auto arbitration_id_type = FDCAN_STANDARD_ID;

    static auto convert_config(CanFilterConfig config) -> uint32_t;

    static auto convert_type(CanFilterType type) -> uint32_t;

    static auto convert_size(uint32_t size) -> uint32_t;
};

template <can_parse::Serializable Message>
void HalCanBus::send(uint32_t arbitration_id, const Message& message) {
    auto length = message.serialize(tx_buffer.begin(), tx_buffer.end());

    tx_header.Identifier = arbitration_id;
    tx_header.DataLength = HalCanBus::convert_size(length);
    tx_header.MessageMarker = 0;

    HAL_FDCAN_AddMessageToTxFifoQ(handle, &tx_header, tx_buffer.data());
}
