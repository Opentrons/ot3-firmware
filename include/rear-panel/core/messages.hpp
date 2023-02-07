#pragma once
#include <cstdint>
#include <variant>

#include "rear-panel/core/binary_parse.hpp"
#include "rear-panel/core/bin_msg_ids.hpp"
#include "common/core/bit_utils.hpp"

namespace messages {

struct IncomingMessageFromHost {
    const uint8_t* buffer;
    const uint8_t* limit;
};


template <MessageType MType>
struct BinaryFormatMessage {
    static const auto message_type = MType;
    auto operator==(const BinaryFormatMessage& other) const -> bool = default;
};

struct Echo : BinaryFormatMessage<MessageType::ECHO> {
    const uint16_t length;
    const uint8_t* data;
    
    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> Echo {
        MessageType type;
        uint16_t len;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (body + len > limit) {
            // we got something wrong from this figure out what we want to
            // do in this situation
            printf("error");
            len = uint16_t(limit - body);
        }
        return Echo{.length = len, .data = body};
    }
    
    auto operator==(const Echo& other) const -> bool = default;
};

using HostCommTaskMessage =
    std::variant<std::monostate, IncomingMessageFromHost, Echo>;

};  // namespace messages
