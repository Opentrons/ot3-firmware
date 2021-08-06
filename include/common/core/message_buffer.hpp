#pragma once

#include <cstdint>

namespace message_buffer {

/**
 * A concept describing a message buffer.
 *
 * @tparam T type constrained by this concept
 */
template <typename T>
concept MessageBuffer = requires(T t, uint8_t* buffer, const uint8_t* cbuffer,
                                 std::size_t buffer_length, uint32_t timeout) {
    { t.send(cbuffer, buffer_length, timeout) } -> std::same_as<std::size_t>;
    { t.send_from_isr(cbuffer, buffer_length) } -> std::same_as<std::size_t>;
    { t.receive(buffer, buffer_length, timeout) } -> std::same_as<std::size_t>;
};

}  // namespace message_buffer
