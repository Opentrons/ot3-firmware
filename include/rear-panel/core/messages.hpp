#pragma once
#include <cstdint>
#include <variant>

#include "common/core/bit_utils.hpp"
#include "common/core/version.h"
#include "rear-panel/core/bin_msg_ids.hpp"
#include "rear-panel/core/binary_parse.hpp"

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
        uint16_t type = 0;
        uint16_t len = 0;
        std::array<uint8_t, MAX_MESSAGE_SIZE> data{};
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (body + len > limit) {
            // we got something wrong from this figure out what we want to
            // do in this situation
            len = uint16_t(limit - body);
        }
        len = std::min(static_cast<size_t>(len), data.size());
        std::copy_n(body, len, data.begin());
        return Echo{.length = len, .data = data};
    }

    auto operator==(const Echo& other) const -> bool = default;
};

struct DeviceInfoRequest : BinaryFormatMessage<MessageType::DEVICE_INFO_REQ> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit) -> DeviceInfoRequest {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            // we got something wrong from this figure out what we want to
            // do in this situation
            len = 0;
        }
        return DeviceInfoRequest{.length = len};
    }

    auto operator==(const DeviceInfoRequest& other) const -> bool = default;
};

struct DeviceInfoResponse : BinaryFormatMessage<MessageType::DEVICE_INFO_RESP> {
    uint16_t length;
    uint32_t version;
    uint32_t flags;
    std::array<char, VERSION_SHORTSHA_SIZE> shortsha;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter = bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(version, iter, limit);
        iter = bit_utils::int_to_bytes(flags, iter, limit);
        iter =
            std::copy_n(&shortsha[0],
                        std::min(limit - iter,
                                 static_cast<ptrdiff_t>(VERSION_SHORTSHA_SIZE)),
                        iter);

        return iter;
    }
    auto operator==(const DeviceInfoResponse& other) const -> bool = default;
};

using HostCommTaskMessage = std::variant<std::monostate, Echo, DeviceInfoRequest>;

static auto rear_panel_parser = binary_parse::Parser<Echo, DeviceInfoRequest>{};

};  // namespace messages
