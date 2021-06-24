#pragma once
#include <array>
#include <variant>

namespace messages {

constexpr size_t RESPONSE_LENGTH = 128;

struct SendResponseMessage {
    std::array<char, RESPONSE_LENGTH> msg;
};

using HostCommsMessage = ::std::variant<SendResponseMessage>;
};  // namespace messages
