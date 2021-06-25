#pragma once

#include <array>
#include <concepts>
#include <span>

#include "pipette_messages.h"
#include "bit_utils.hpp"

template <class C>
concept ReaderProtocol = requires(C c, int l, uint8_t* buff) {
    {c.recv(l, buff)};
};


class MessageReader {
  public:
    MessageReader() = default;

    template <ReaderProtocol reader>
    auto read_command(reader& communication)
        -> pipette_messages::ReceivedMessage;

  private:
    static constexpr auto max_payload_length = 8;
    std::array<uint8_t, max_payload_length> payload_buffer{};
};


template <ReaderProtocol reader>
auto MessageReader::read_command(reader& communication)
    -> pipette_messages::ReceivedMessage {
    pipette_messages::ReceivedMessage r;

    uint8_t length = 0;
    // Read the length
    communication.recv(1, &length);

    // Read the remainder
    communication.recv(length, payload_buffer.data());

    auto span = std::span{payload_buffer};

    uint32_t arbitration_id = 0;
    bit_utils::bytes_to_int(span.subspan(0, 4), arbitration_id);

    switch (arbitration_id) {
        case static_cast<uint32_t>(pipette_messages::MessageType::stop):
            r = pipette_messages::Stop{};
            break;
        case static_cast<uint32_t>(pipette_messages::MessageType::get_speed):
            r = pipette_messages::GetSpeed{};
            break;
        case static_cast<uint32_t>(pipette_messages::MessageType::set_speed): {
            uint32_t speed = 0;
            bit_utils::bytes_to_int(span.subspan(4, 4), speed);
            r = pipette_messages::SetSpeed{speed};
            break;
        }
        default:
            break;
    }
    return r;
}