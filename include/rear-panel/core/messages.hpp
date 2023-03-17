#pragma once
#include <algorithm>
#include <cstdint>
#include <variant>

#include "common/core/bit_utils.hpp"
#include "common/core/version.h"
#include "rear-panel/core/bin_msg_ids.hpp"
#include "rear-panel/core/binary_parse.hpp"

// this is the max size of the double_buffer used in freertos_message_queue
constexpr size_t MAX_MESSAGE_SIZE = 512U * 2;
namespace rearpanel {
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
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, Echo> {
        uint16_t type = 0;
        uint16_t len = 0;
        std::array<uint8_t, MAX_MESSAGE_SIZE> data{};
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (body + len > limit) {
            return std::monostate{};
        }
        len = std::min(static_cast<size_t>(len), data.size());
        std::copy_n(body, len, data.begin());
        return Echo{.length = len, .data = data};
    }

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = std::copy_n(data.begin(), length, iter);
        return iter;
    }

    auto operator==(const Echo& other) const -> bool = default;
};

struct Ack : BinaryFormatMessage<MessageType::ACK> {
    uint16_t length;
    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        return iter;
    }
};

struct AckFailed : BinaryFormatMessage<MessageType::ACK_FAILED> {
    uint16_t length;
    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        return iter;
    }
};

struct DeviceInfoRequest : BinaryFormatMessage<MessageType::DEVICE_INFO_REQ> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, DeviceInfoRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
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
    char primary_revision;
    char secondary_revision;
    std::array<char, 2> tertiary_revision;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(version, iter, limit);
        iter = bit_utils::int_to_bytes(flags, iter, limit);
        iter =
            std::copy_n(&shortsha[0],
                        std::min(limit - iter,
                                 static_cast<ptrdiff_t>(VERSION_SHORTSHA_SIZE)),
                        iter);
        iter = std::copy_n(
            &primary_revision,
            std::min(limit - iter, ptrdiff_t(sizeof(primary_revision))), iter);
        iter = std::copy_n(
            &secondary_revision,
            std::min(limit - iter, ptrdiff_t(sizeof(secondary_revision))),
            iter);
        iter = std::copy_n(
            &tertiary_revision[0],
            std::min(limit - iter, ptrdiff_t(sizeof(tertiary_revision))), iter);
        return iter;
    }

    static auto get_length() -> uint16_t {
        return sizeof(uint32_t) + sizeof(uint32_t) + VERSION_SHORTSHA_SIZE +
               sizeof(revision);
    }
    auto operator==(const DeviceInfoResponse& other) const -> bool = default;
};

struct EnterBootloader : BinaryFormatMessage<MessageType::ENTER_BOOTLOADER> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, EnterBootloader> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return EnterBootloader{.length = len};
    }

    auto operator==(const EnterBootloader& other) const -> bool = default;
};

struct EnterBootloaderResponse
    : BinaryFormatMessage<MessageType::ENTER_BOOTLOADER_RESPONSE> {
    uint16_t length = sizeof(bool);
    bool success;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(success, iter, limit);
        return iter;
    }
    auto operator==(const EnterBootloaderResponse& other) const
        -> bool = default;
};

struct EngageEstopRequest
    : BinaryFormatMessage<MessageType::ENGAGE_ESTOP_REQUEST> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, EngageEstopRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return EngageEstopRequest{.length = len};
    }

    auto operator==(const EngageEstopRequest& other) const -> bool = default;
};

struct ReleaseEstopRequest
    : BinaryFormatMessage<MessageType::RELEASE_ESTOP_REQUEST> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, ReleaseEstopRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return ReleaseEstopRequest{.length = len};
    }

    auto operator==(const ReleaseEstopRequest& other) const -> bool = default;
};

struct EngageSyncRequest
    : BinaryFormatMessage<MessageType::ENGAGE_SYNC_REQUEST> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, EngageSyncRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return EngageSyncRequest{.length = len};
    }

    auto operator==(const EngageSyncRequest& other) const -> bool = default;
};

struct ReleaseSyncRequest
    : BinaryFormatMessage<MessageType::RELEASE_SYNC_REQUEST> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, ReleaseSyncRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return ReleaseSyncRequest{.length = len};
    }

    auto operator==(const ReleaseSyncRequest& other) const -> bool = default;
};
// HostCommTaskMessage list must be a superset of the messages in the parser
using HostCommTaskMessage =
    std::variant<std::monostate, Echo, DeviceInfoRequest, Ack, AckFailed,
                 EnterBootloader, EnterBootloaderResponse, EngageEstopRequest,
                 EngageSyncRequest, ReleaseEstopRequest, ReleaseSyncRequest>;

using SystemTaskMessage =
    std::variant<std::monostate, EnterBootloader, EngageEstopRequest,
                 EngageSyncRequest, ReleaseEstopRequest, ReleaseSyncRequest>;

using LightControlTaskMessage =
    std::variant<std::monostate>;

static auto rear_panel_parser =
    binary_parse::Parser<Echo, DeviceInfoRequest, EnterBootloader,
                         EngageEstopRequest, EngageSyncRequest,
                         ReleaseEstopRequest, ReleaseSyncRequest>{};

};  // namespace messages
};  // namespace rearpanel
