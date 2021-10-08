#pragma once

#include <array>
#include <span>

#include "common/core/bit_utils.hpp"
#include "common/core/io.hpp"
#include "gantry_messages.hpp"

using namespace io;

namespace gantry_communication {

class MessageReader {
  public:
    MessageReader() = default;

    template <ReaderProtocol Reader>
    auto read(Reader &communication) -> gantry_messages::ReceivedMessage;

  private:
    static constexpr auto max_payload_length = 8;
    std::array<uint8_t, max_payload_length> payload_buffer{};
    std::span<uint8_t> payload_span{payload_buffer};
};

template <ReaderProtocol Reader>
auto MessageReader::read(Reader &communication)
    -> gantry_messages::ReceivedMessage {
    gantry_messages::ReceivedMessage r;

    auto arbitration_id_span = payload_span.subspan(0, 4);

    // Read the arbitration id
    communication.read(arbitration_id_span);

    uint32_t arbitration_id = 0;
    bit_utils::bytes_to_int(arbitration_id_span, arbitration_id);

    switch (arbitration_id) {
        case static_cast<uint32_t>(gantry_messages::MessageType::stop):
            return gantry_messages::Stop{};
        case static_cast<uint32_t>(gantry_messages::MessageType::setup):
            return gantry_messages::Setup{};
        case static_cast<uint32_t>(gantry_messages::MessageType::move): {
            // Read the steps
            auto mm_span = payload_span.subspan(0, 4);
            communication.read(mm_span);

            int32_t mm = 0;
            bit_utils::bytes_to_int(mm_span, mm);

            return motor_messages::CanMove{mm};
        }
        case static_cast<uint32_t>(gantry_messages::MessageType::status):
            return gantry_messages::Status{};
        case static_cast<uint32_t>(gantry_messages::MessageType::get_speed):
            return gantry_messages::GetSpeed{};
        case static_cast<uint32_t>(gantry_messages::MessageType::set_speed): {
            // Read the speed
            auto speed_span = payload_span.subspan(0, 4);
            communication.read(speed_span);

            uint32_t speed = 0;
            bit_utils::bytes_to_int(speed_span, speed);

            return gantry_messages::SetSpeed{speed};
        }
        default:
            return r;
    }
}

class MessageWriter {
  public:
    MessageWriter() = default;

    template <WriterProtocol Writer>
    void write(Writer &communication, const gantry_messages::SentMessage &m);

  private:
    template <typename Iter, typename Limit>
    requires std::forward_iterator<Iter>
    auto write(Iter iter, const Limit limit,
               const gantry_messages::GetSpeedResult &m) -> Iter {
        iter = bit_utils::int_to_bytes(
            static_cast<uint32_t>(
                gantry_messages::MessageType::get_speed_result),
            iter, limit);
        return bit_utils::int_to_bytes(m.mm_sec, iter, limit);
    }

    template <typename Iter, typename Limit>
    requires std::forward_iterator<Iter>
    auto write(Iter iter, const Limit limit,
               const gantry_messages::GetStatusResult &m) -> Iter {
        iter = bit_utils::int_to_bytes(
            static_cast<uint32_t>(
                gantry_messages::MessageType::get_status_result),
            iter, limit);
        iter = bit_utils::int_to_bytes(m.status, iter, limit);
        iter = bit_utils::int_to_bytes(m.data, iter, limit);
        return iter;
    }

    template <typename Iter, typename Limit>
    requires std::forward_iterator<Iter>
    auto write(Iter iter, Limit limit, const std::monostate &m) -> Iter {
        static_cast<void>(m);
        static_cast<void>(limit);
        return iter;
    }

    static constexpr auto max_payload_length = 8;
    std::array<uint8_t, max_payload_length> payload_buffer{};
};

template <WriterProtocol Writer>
void MessageWriter::write(Writer &communication,
                          const gantry_messages::SentMessage &m) {
    auto *iter = payload_buffer.begin();

    auto visitor = [&iter, this](auto val) {
        iter = this->write(iter, payload_buffer.end(), val);
    };

    std::visit(visitor, m);

    auto subspan = std::span(payload_buffer.begin(), iter);

    communication.write(subspan);
}
}  // namespace gantry_communication
