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


class MessageWriter {
public:
    MessageWriter() = default;

    template<WriterProtocol Writer>
    void write(Writer &communication, const pipette_messages::SentMessage & m);

private:
    template<typename Iter>
    requires std::forward_iterator<Iter>
    auto write(Iter iter, const pipette_messages::GetSpeedResult & m) -> Iter {
        iter = bit_utils::int_to_bytes(static_cast<uint32_t>(pipette_messages::MessageType::get_speed_result), iter);
        return bit_utils::int_to_bytes(m.mm_sec, iter);
    }

    template<typename Iter> requires std::forward_iterator<Iter>
    auto write(Iter iter, const std::monostate & m) -> Iter {
        static_cast<void>(m);
        return iter;
    }

    static constexpr auto max_payload_length = 8;
    std::array<uint8_t, max_payload_length> payload_buffer{};
};

template<WriterProtocol Writer>
void MessageWriter::write(Writer &communication, const pipette_messages::SentMessage & m) {
    auto * iter = payload_buffer.begin();

    auto visitor = [&iter, this](auto val) {iter = this->write(iter, val);};

    std::visit(visitor, m);

    auto subspan = std::span(payload_buffer.begin(), iter);

    communication.write(subspan);
}
}