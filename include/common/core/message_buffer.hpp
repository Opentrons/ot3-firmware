#pragma once

#include <array>
#include <cstdint>

namespace message_buffer {

/**
 * A concept describing a message buffer.
 *
 * @tparam T type constrained by this concept
 */
template <typename T>
concept MessageBuffer = requires(T t, std::array<uint8_t, 0> buffer,
                                 uint32_t timeout) {
    {
        t.send(buffer.cbegin(), buffer.cend(), timeout)
        } -> std::same_as<std::size_t>;
    {
        t.send_from_isr(buffer.cbegin(), buffer.cend())
        } -> std::same_as<std::size_t>;
    {
        t.receive(buffer.begin(), buffer.end(), timeout)
        } -> std::same_as<std::size_t>;
};

}  // namespace message_buffer
