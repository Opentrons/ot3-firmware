#pragma once

#include <array>

#include "can/core/message_core.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/message_buffer.hpp"

namespace can_message_buffer {

using namespace message_buffer;

/**
 * Class to write can arbitration ids and data payload into a MessageBuffer.
 *
 * @tparam Buffer A MessageBuffer instance.
 */
template <MessageBuffer Buffer>
class CanMessageBufferWriter {
  public:
    explicit CanMessageBufferWriter(Buffer& b) : message_buffer{b} {}

    /**
     * Write a can message to the message buffer.
     *
     * @tparam Input Iterator iterator into the payload of message
     * @tparam Limit End type of payload of message
     * @param arbitration_id the arbitration id
     * @param buffer Payload iterator
     * @param limit End of payload
     * @param timeout timeout
     * @return True on success.
     */
    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sized_sentinel_for<Limit, Input>
    auto send(uint32_t arbitration_id, Input buffer, Limit limit,
              uint32_t timeout) -> bool {
        uint32_t num_bytes = write_to_backing(arbitration_id, buffer, limit);
        return message_buffer.send(backing.begin(), backing.begin() + num_bytes,
                                   timeout) == num_bytes;
    }

    /**
     * Write a can message to the message buffer from an ISR.
     *
     * @tparam Input Iterator iterator into the payload of message
     * @tparam Limit End type of payload of message
     * @param arbitration_id the arbitration id
     * @param buffer Payload iterator
     * @param limit End of payload
     * @return True on success.
     */
    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sized_sentinel_for<Limit, Input>
    auto send_from_isr(uint32_t arbitration_id, Input buffer, Limit limit)
        -> bool {
        uint32_t num_bytes = write_to_backing(arbitration_id, buffer, limit);
        return message_buffer.send_from_isr(
                   backing.begin(), backing.begin() + num_bytes) == num_bytes;
    }

  private:
    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sized_sentinel_for<Limit, Input>
    auto write_to_backing(uint32_t arbitration_id, Input buffer, Limit limit)
        -> uint32_t {
        auto* iter = bit_utils::int_to_bytes(arbitration_id, backing.begin(),
                                             backing.end());
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (; iter != backing.end() && buffer != limit; iter++, buffer++) {
            *iter = *buffer;
        }
        return iter - backing.begin();
    }

    static auto constexpr buffer_size =
        message_core::MaxMessageSize + sizeof(uint32_t);
    std::array<uint8_t, buffer_size> backing{};
    Buffer& message_buffer;
};

/**
 * The CanMessageBufferReader callback interface
 *
 * @tparam T type constrained by this concept
 */
template <typename T>
concept CanMessageBufferListener = requires(T t, uint32_t arbitration_id,
                                            std::array<uint8_t, 0> arr) {
    {t.handle(arbitration_id, arr.begin(), arr.end())};
};

/**
 * Class that reads a can message's arbitration id and data payload from a
 * MessageBuffer and notifies a CanMessageBufferListener
 *
 * @tparam Buffer the MessageBuffer type
 * @tparam Listener the CanMessageBufferListener type
 */
template <MessageBuffer Buffer, CanMessageBufferListener Listener>
class CanMessageBufferReader {
  public:
    /**
     * Constructor
     * @param b A MessageBuffer instance to read from
     * @param l The CanMessageBufferListener to notify on a read
     */
    explicit CanMessageBufferReader(Buffer& b, Listener& l)
        : message_buffer{b}, listener{l} {}

    /**
     * Read a message from the message buffer and notify listener.
     * @param timeout
     */
    auto read(uint32_t timeout) -> void {
        auto read_amount =
            message_buffer.receive(backing.begin(), backing.end(), timeout);
        if (read_amount > 0) {
            uint32_t arbitration_id = 0;
            auto* end = backing.begin() + read_amount;
            auto* start_of_message = bit_utils::bytes_to_int(
                backing.begin(), backing.end(), arbitration_id);
            listener.handle(arbitration_id, start_of_message, end);
        }
    }

  private:
    static auto constexpr buffer_size =
        message_core::MaxMessageSize + sizeof(uint32_t);
    std::array<uint8_t, buffer_size> backing{};
    Buffer& message_buffer;
    Listener& listener;
};

}  // namespace can_message_buffer
