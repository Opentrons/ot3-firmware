#pragma once

#include <array>
#include <span>

#include "common/hal/io.hpp"
#include "bit_utils.hpp"
#include "pipette_messages.h"

using namespace io;

namespace communication {

class MessageReader {
public:
    MessageReader() = default;

    template<ReaderProtocol Reader>
    auto read(Reader &communication)
    -> pipette_messages::ReceivedMessage;

private:
    static constexpr auto max_payload_length = 8;
    std::array<uint8_t, max_payload_length> payload_buffer{};
    std::span<uint8_t> payload_span{payload_buffer};
};

template<ReaderProtocol Reader>
auto MessageReader::read(Reader &communication)
-> pipette_messages::ReceivedMessage {
    pipette_messages::ReceivedMessage r;

    auto arbitration_id_span = payload_span.subspan(0, 4);

    // Read the arbitration id
    communication.read(arbitration_id_span);

    uint32_t arbitration_id = 0;
    bit_utils::bytes_to_int(arbitration_id_span, arbitration_id);

    switch (arbitration_id) {
        case static_cast<uint32_t>(pipette_messages::MessageType::stop):
            r = pipette_messages::Stop{};
            break;
        case static_cast<uint32_t>(pipette_messages::MessageType::get_speed):
            r = pipette_messages::GetSpeed{};
            break;
        case static_cast<uint32_t>(pipette_messages::MessageType::set_speed): {
            // Read the speed
            auto speed_span = payload_span.subspan(0, 4);
            communication.read(speed_span);

            uint32_t speed = 0;
            bit_utils::bytes_to_int(speed_span, speed);

            r = pipette_messages::SetSpeed{speed};
            break;
        }
        default:
            break;
    }
    return r;
}

}