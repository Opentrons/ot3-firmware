
#include "can/firmware/hal_can_message_buffer.hpp"

#include <array>

#include "can/core/can_message_buffer.hpp"
#include "can/core/message_core.hpp"
#include "can/firmware/utils.hpp"
#include "stm32g4xx_hal_conf.h"

using namespace hal_can_message_buffer;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto read_can_message_buffer = ReadMessageBuffer{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto read_can_message_buffer_writer =
    can_message_buffer::CanMessageBufferWriter(read_can_message_buffer);

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static FDCAN_RxHeaderTypeDef RxHeader;
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static auto buff = std::array<uint8_t, message_core::MaxMessageSize>{};

/**
 * Read from fifo and write read message to the read_can_message_buffer.
 *
 * @param hfdcan pointer to a can handle
 * @param fifo either FDCAN_RX_FIFO0 or FDCAN_RX_FIFO1
 */
static void try_read_from_fifo(FDCAN_HandleTypeDef *hfdcan, uint32_t fifo) {
    if (HAL_FDCAN_GetRxFifoFillLevel(hfdcan, fifo) > 0) {
        if (HAL_FDCAN_GetRxMessage(hfdcan, fifo, &RxHeader, buff.data()) ==
            HAL_OK) {
            // Convert the length from HAL to integer
            auto data_length = static_cast<uint32_t>(
                hal_can_utils::length_from_hal(RxHeader.DataLength));

            read_can_message_buffer_writer.send_from_isr(
                RxHeader.Identifier, buff.begin(), buff.begin() + data_length);
        }
    }
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0) {
        try_read_from_fifo(hfdcan, FDCAN_RX_FIFO0);
    }
}

void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo1ITs) {
    if ((RxFifo1ITs & FDCAN_IT_RX_FIFO1_NEW_MESSAGE) != 0) {
        try_read_from_fifo(hfdcan, FDCAN_RX_FIFO1);
    }
}

auto hal_can_message_buffer::get_message_buffer() -> ReadMessageBuffer & {
    return read_can_message_buffer;
}