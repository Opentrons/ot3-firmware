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

template <rearpanel::ids::BinaryMessageId MType>
struct BinaryFormatMessage {
    static const auto message_type = MType;
    auto operator==(const BinaryFormatMessage& other) const -> bool = default;
};

struct Echo : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::echo> {
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

struct Ack : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::ack> {
    uint16_t length;
    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        return iter;
    }
};

struct AckFailed
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::ack_failed> {
    uint16_t length;
    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        return iter;
    }
};

struct DeviceInfoRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::device_info_request> {
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

// Empty message sent periodically to drive timing of the light task
struct UpdateLightControlMessage {};

struct DeviceInfoResponse
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::device_info_response> {
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
        // This is only used for the rear panel and there is not currently a
        // subid required there
        const uint8_t subid = 0;
        iter = bit_utils::int_to_bytes(subid, iter, limit);
        return iter;
    }

    static auto get_length() -> uint16_t {
        return sizeof(uint32_t) + sizeof(uint32_t) + VERSION_SHORTSHA_SIZE +
               revision_size() + sizeof(uint8_t);
    }
    auto operator==(const DeviceInfoResponse& other) const -> bool = default;
};

struct EnterBootloader
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::enter_bootloader_request> {
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

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct EnterBootloaderResponse
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::enter_bootloader_response> {
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
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::engage_estop> {
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
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::release_estop> {
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
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::engage_nsync_out> {
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
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::release_nsync_out> {
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

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct EstopStateChange
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::estop_state_change> {
    uint16_t length = sizeof(bool);
    bool engaged;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(engaged, iter, limit);
        return iter;
    }
    auto operator==(const EstopStateChange& other) const -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct EstopButtonDetectionChange
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::estop_button_detection_change> {
    uint16_t length = 2 * sizeof(bool);
    bool aux1_present;
    bool aux2_present;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(aux1_present, iter, limit);
        iter = bit_utils::int_to_bytes(aux2_present, iter, limit);
        return iter;
    }
    auto operator==(const EstopButtonDetectionChange& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct EstopButtonPresentRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::estop_button_present_request> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, EstopButtonPresentRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return EstopButtonPresentRequest{.length = len};
    }

    auto operator==(const EstopButtonPresentRequest& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct AuxPortDetectionChange
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::aux_present_detection_change> {
    uint16_t length = 2 * sizeof(bool);
    bool aux1_present;
    bool aux2_present;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(aux1_present, iter, limit);
        iter = bit_utils::int_to_bytes(aux2_present, iter, limit);
        return iter;
    }
    auto operator==(const AuxPortDetectionChange& other) const
        -> bool = default;
};

struct AuxPresentRequeset
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::aux_present_request> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, AuxPresentRequeset> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return AuxPresentRequeset{.length = len};
    }

    auto operator==(const AuxPresentRequeset& other) const -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct AuxIDResponse
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::aux_id_response> {
    uint16_t length = 2 * sizeof(bool);
    bool aux1_id_state;
    bool aux2_id_state;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(aux1_id_state, iter, limit);
        iter = bit_utils::int_to_bytes(aux2_id_state, iter, limit);
        return iter;
    }
    auto operator==(const AuxIDResponse& other) const -> bool = default;
};

struct AuxIDRequest
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::aux_id_request> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, AuxIDRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return AuxIDRequest{.length = len};
    }

    auto operator==(const AuxIDRequest& other) const -> bool = default;
};

struct DoorSwitchStateRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::door_switch_state_request> {
    uint16_t length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, DoorSwitchStateRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (len > 0) {
            return std::monostate{};
        }
        return DoorSwitchStateRequest{.length = len};
    }

    auto operator==(const DoorSwitchStateRequest& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct DoorSwitchStateInfo
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::door_switch_state_info> {
    uint16_t length = sizeof(bool);
    bool open;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(open, iter, limit);
        return iter;
    }
    auto operator==(const DoorSwitchStateInfo& other) const -> bool = default;
};

struct WriteEEPromRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::write_eeprom_request> {
    uint16_t length;
    uint16_t data_address;
    uint16_t data_length;
    std::array<uint8_t, 8> data;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, WriteEEPromRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        uint16_t data_addr = 0;
        uint16_t data_len = 0;
        std::array<uint8_t, 8> data{};
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (body + len > limit) {
            return std::monostate{};
        }
        body = bit_utils::bytes_to_int(body, limit, data_addr);
        body = bit_utils::bytes_to_int(body, limit, data_len);

        std::copy_n(body, data_len, data.begin());
        return WriteEEPromRequest{.length = len,
                                  .data_address = data_addr,
                                  .data_length = data_len,
                                  .data = data};
    }

    auto operator==(const WriteEEPromRequest& other) const -> bool = default;
};

struct ReadEEPromRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::read_eeprom_request> {
    uint16_t length;
    uint16_t data_address;
    uint16_t data_length;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, ReadEEPromRequest> {
        uint16_t type = 0;
        uint16_t len = 0;
        uint16_t data_addr = 0;
        uint16_t data_len = 0;
        body = bit_utils::bytes_to_int(body, limit, type);
        body = bit_utils::bytes_to_int(body, limit, len);
        if (body + len > limit) {
            return std::monostate{};
        }
        body = bit_utils::bytes_to_int(body, limit, data_addr);
        body = bit_utils::bytes_to_int(body, limit, data_len);
        return ReadEEPromRequest{
            .length = len, .data_address = data_addr, .data_length = data_len};
    }

    auto operator==(const ReadEEPromRequest& other) const -> bool = default;
};

struct ReadEEPromResponse
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::read_eeprom_response> {
    uint16_t length = 2 * sizeof(uint16_t) + 8;
    uint16_t data_address;
    uint16_t data_length;
    std::array<uint8_t, 8> data;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(length, iter, limit);
        iter = bit_utils::int_to_bytes(data_address, iter, limit);
        iter = bit_utils::int_to_bytes(data_length, iter, limit);
        iter = std::copy_n(data.begin(), data_length, iter);
        return iter;
    }

    auto operator==(const ReadEEPromResponse& other) const -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct AddLightActionRequest
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::add_light_action> {
    static constexpr size_t LENGTH =
        sizeof(uint16_t) + sizeof(rearpanel::ids::LightTransitionType) +
        (4 * sizeof(uint8_t));
    uint16_t length;
    uint16_t transition_time_ms;
    rearpanel::ids::LightTransitionType transition;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t white;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, AddLightActionRequest> {
        uint16_t type = 0;
        uint8_t transition_buf = 0;
        auto ret = AddLightActionRequest{
            .length = 0,
            .transition_time_ms = 0,
            .transition = rearpanel::ids::LightTransitionType::linear,
            .red = 0,
            .green = 0,
            .blue = 0,
            .white = 0};

        body = bit_utils::bytes_to_int(body, limit, type);
        if (type != static_cast<uint16_t>(message_type)) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, ret.length);
        if (body == limit) {
            return std::monostate();
        }
        if (ret.length != LENGTH) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, ret.transition_time_ms);
        if (body == limit) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, transition_buf);
        ret.transition =
            static_cast<rearpanel::ids::LightTransitionType>(transition_buf);
        if (body == limit) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, ret.red);
        if (body == limit) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, ret.green);
        if (body == limit) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, ret.blue);
        if (body == limit) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, ret.white);
        return ret;
    }

    auto operator==(const AddLightActionRequest& other) const -> bool = default;
};

struct ClearLightActionStagingQueueRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::clear_light_action_staging_queue> {
    static constexpr size_t LENGTH = 0;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, ClearLightActionStagingQueueRequest> {
        uint16_t type = 0;
        uint16_t length = 0;

        body = bit_utils::bytes_to_int(body, limit, type);
        if (type != static_cast<uint16_t>(message_type)) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, length);
        if (length != LENGTH) {
            return std::monostate();
        }
        return ClearLightActionStagingQueueRequest{};
    }

    auto operator==(const ClearLightActionStagingQueueRequest& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct StartLightActionRequest
    : BinaryFormatMessage<rearpanel::ids::BinaryMessageId::start_light_action> {
    static constexpr size_t LENGTH = sizeof(rearpanel::ids::LightAnimationType);
    rearpanel::ids::LightAnimationType animation;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, StartLightActionRequest> {
        uint16_t type = 0;
        uint16_t length = 0;
        uint8_t animation = 0;

        body = bit_utils::bytes_to_int(body, limit, type);
        if (type != static_cast<uint16_t>(message_type)) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, length);
        if (length != LENGTH) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, animation);
        return StartLightActionRequest{
            .animation =
                static_cast<rearpanel::ids::LightAnimationType>(animation)};
    }

    auto operator==(const StartLightActionRequest& other) const
        -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct SetDeckLightRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::set_deck_light_request> {
    static constexpr size_t LENGTH = sizeof(uint8_t);
    uint8_t setting;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, SetDeckLightRequest> {
        uint16_t type = 0;
        uint16_t length = 0;
        uint8_t setting = 0;

        body = bit_utils::bytes_to_int(body, limit, type);
        if (type != static_cast<uint16_t>(message_type)) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, length);
        if (length != LENGTH) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, setting);
        return SetDeckLightRequest{.setting = setting};
    }

    auto operator==(const SetDeckLightRequest& other) const -> bool = default;
};

struct GetDeckLightRequest
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::get_deck_light_request> {
    static constexpr size_t LENGTH = 0;

    template <bit_utils::ByteIterator Input, typename Limit>
    static auto parse(Input body, Limit limit)
        -> std::variant<std::monostate, GetDeckLightRequest> {
        uint16_t type = 0;
        uint16_t length = 0;

        body = bit_utils::bytes_to_int(body, limit, type);
        if (type != static_cast<uint16_t>(message_type)) {
            return std::monostate();
        }
        body = bit_utils::bytes_to_int(body, limit, length);
        if (length != LENGTH) {
            return std::monostate();
        }
        return GetDeckLightRequest{};
    }

    auto operator==(const GetDeckLightRequest& other) const -> bool = default;
};

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
struct GetDeckLightResponse
    : BinaryFormatMessage<
          rearpanel::ids::BinaryMessageId::get_deck_light_response> {
    static constexpr uint16_t LENGTH = sizeof(uint8_t);
    uint8_t setting;

    template <bit_utils::ByteIterator Output, typename Limit>
    auto serialize(Output body, Limit limit) const -> Output {
        auto iter =
            bit_utils::int_to_bytes(uint16_t(message_type), body, limit);
        iter = bit_utils::int_to_bytes(LENGTH, iter, limit);
        iter = bit_utils::int_to_bytes(setting, iter, limit);
        return iter;
    }
    auto operator==(const GetDeckLightResponse& other) const -> bool = default;
};

// HostCommTaskMessage list must be a superset of the messages in the parser
using HostCommTaskMessage = std::variant<
    std::monostate, Echo, DeviceInfoRequest, Ack, AckFailed, EnterBootloader,
    EnterBootloaderResponse, EngageEstopRequest, EngageSyncRequest,
    ReleaseEstopRequest, ReleaseSyncRequest, EstopStateChange,
    EstopButtonDetectionChange, DoorSwitchStateRequest, DoorSwitchStateInfo,
    AddLightActionRequest, ClearLightActionStagingQueueRequest,
    StartLightActionRequest, SetDeckLightRequest, GetDeckLightRequest,
    GetDeckLightResponse, AuxPortDetectionChange, AuxPresentRequeset,
    AuxIDResponse, AuxIDRequest, EstopButtonPresentRequest, WriteEEPromRequest,
    ReadEEPromRequest, ReadEEPromResponse>;

using SystemTaskMessage =
    std::variant<std::monostate, EnterBootloader, EngageEstopRequest,
                 EngageSyncRequest, ReleaseEstopRequest, ReleaseSyncRequest,
                 DoorSwitchStateRequest, AuxPresentRequeset, AuxIDRequest,
                 EstopButtonPresentRequest, ReadEEPromRequest>;

using LightControlTaskMessage =
    std::variant<std::monostate, UpdateLightControlMessage,
                 AddLightActionRequest, ClearLightActionStagingQueueRequest,
                 StartLightActionRequest, SetDeckLightRequest,
                 GetDeckLightRequest>;

using HardwareTaskMessage = std::variant<std::monostate>;

using Parser = binary_parse::Parser<
    Echo, DeviceInfoRequest, EnterBootloader, EngageEstopRequest,
    EngageSyncRequest, ReleaseEstopRequest, ReleaseSyncRequest,
    DoorSwitchStateRequest, AddLightActionRequest,
    ClearLightActionStagingQueueRequest, StartLightActionRequest,
    SetDeckLightRequest, GetDeckLightRequest, AuxPresentRequeset, AuxIDRequest,
    EstopButtonPresentRequest, WriteEEPromRequest, ReadEEPromRequest>;

};  // namespace messages
};  // namespace rearpanel
