#pragma once
#include <cstdint>
#include <variant>

namespace messages {

struct IncomingMessageFromHost {
    const uint8_t* buffer;
    const uint8_t* limit;
};

// TODO(ryan): as part of RET-1307 create an auto gen of the ids like the can
// messages and include them from another file
enum class MessageType : uint16_t {
    ECHO = 0x00,
};

template <MessageType MType>
struct BinaryFormatMessage {
    static const auto message_type = MType;
    const uint16_t length;
    const uint8_t* data;
    auto operator==(const BinaryFormatMessage& other) const -> bool = default;
};

using Echo = BinaryFormatMessage<MessageType::ECHO>;

using HostCommTaskMessage =
    std::variant<std::monostate, IncomingMessageFromHost, Echo>;

};  // namespace messages
