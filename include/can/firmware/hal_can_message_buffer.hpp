#pragma once

#include "can/firmware/hal_can_bus.hpp"
#include "common/core/freertos_message_buffer.hpp"

namespace hal_can_message_buffer {

constexpr auto read_message_buffer_size = 1024;

/**
 * The type of the message buffer populated by HAL ISR.
 */
using ReadMessageBuffer =
    freertos_message_buffer::FreeRTOMessageBuffer<read_message_buffer_size>;

/**
 * Access to the message buffer populated by HAL ISR
 * @return
 */
auto get_message_buffer() -> ReadMessageBuffer&;

}  // namespace hal_can_message_buffer