#pragma once
#include <cstdint>
#include <variant>

#include "rear-panel/core/binary_parse.hpp"
#include "rear-panel/core/bin_msg_ids.hpp"
#include "common/core/bit_utils.hpp"

// this is the max size of the double_buffer used in freertos_message_queue
constexpr size_t MAX_MESSAGE_SIZE = 512U * 4;

namespace messages {


template <MessageType MType>
struct BinaryFormatMessage {
    static const auto message_type = MType;
    auto operator==(const BinaryFormatMessage& other) const -> bool = default;
};

struct Echo : BinaryFormatMessage<MessageType::ECHO> {
    uint16_t length;
    std::array<uint8_t, MAX_MESSAGE_SIZE> data;
    
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> Echo {
        uint16_t type;
        uint16_t len;
        std::array<uint8_t, MAX_MESSAGE_SIZE> data{};
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (body + len > limit) {
            // we got something wrong from this figure out what we want to
            // do in this situation
            printf("error");
            len = uint16_t(limit - body);
        }
        len = std::min(static_cast<size_t>(len), data.size());
        std::copy_n(body, len, data.begin());
        return Echo{.length = len, .data = data};
    }
    
    auto operator==(const Echo& other) const -> bool = default;
};


using HostCommTaskMessage =
    std::variant<std::monostate, Echo>;
    
static auto rear_panel_parser = binary_parse::Parser<Echo>{};


};  // namespace messages
