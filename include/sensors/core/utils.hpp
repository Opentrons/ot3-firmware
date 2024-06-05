#pragma once

#include <ranges>

#include "can/core/messages.hpp"
#include "common/core/message_utils.hpp"
#include "i2c/core/messages.hpp"

namespace sensors {

namespace tip_presence {
struct TipStatusChangeDetected {};

using TaskMessage = std::variant<std::monostate, TipStatusChangeDetected,
                                 can::messages::TipStatusQueryRequest>;

}  // namespace tip_presence

namespace utils {

using CanMessageTuple =
    std::tuple<can::messages::ReadFromSensorRequest,
               can::messages::SendAccumulatedSensorDataRequest,
               can::messages::WriteToSensorRequest,
               can::messages::BaselineSensorRequest,
               can::messages::SetSensorThresholdRequest,
               can::messages::BindSensorOutputRequest,
               can::messages::PeripheralStatusRequest>;
using OtherTaskMessagesTuple = std::tuple<i2c::messages::TransactionResponse>;
using CanMessageHandler = typename ::utils::TuplesToVariants<
    std::tuple<std::monostate, can::messages::TipStatusQueryRequest>,
    CanMessageTuple>::type;
using TaskMessage = typename ::utils::VariantCat<
    std::variant<std::monostate>,
    typename ::utils::TuplesToVariants<CanMessageTuple,
                                       OtherTaskMessagesTuple>::type>::type;

/**
 * Concept describing a class that can message this task.
 * @tparam Client
 */
template <typename Client>
concept TaskClient = requires(Client client, const TaskMessage& m) {
    {client.send_capacitive_sensor_queue_front(m)};
};

enum class BitMode : uint8_t { LSB = 0x0, MSB = 0x1 };

// Bit positions to pack in an 8 bit response tag
enum class ResponseTag : size_t {
    IS_PART_OF_POLL = 0,
    IS_BASELINE = 1,
    POLL_IS_CONTINUOUS = 2,
    IS_THRESHOLD_SENSE = 4,
};

[[nodiscard]] constexpr auto byte_from_tag(ResponseTag tag) -> uint8_t {
    return (1 << static_cast<size_t>(tag));
}

template <std::ranges::range Tags>
[[nodiscard]] auto byte_from_tags(const Tags& tags) -> uint8_t {
    uint8_t result = 0;
    for (auto tag : tags) {
        result |= byte_from_tag(tag);
    }
    return result;
}

[[nodiscard]] inline constexpr auto tag_in_token(uint32_t token,
                                                 ResponseTag tag) -> bool {
    return bool(token & (1 << static_cast<size_t>(tag)));
}

[[nodiscard]] inline constexpr auto tags_from_token(uint32_t token) -> uint8_t {
    return static_cast<uint8_t>(token & 0xff);
}

[[nodiscard]] inline constexpr auto build_id(uint16_t address, uint8_t reg,
                                             uint8_t tags = 0) -> uint32_t {
    return (static_cast<uint32_t>(address) << 16) |
           (static_cast<uint32_t>(reg) << 8) | (tags);
}

template <typename RegType>
[[nodiscard]] inline constexpr auto reg_from_id(uint32_t id) -> RegType {
    return static_cast<RegType>(static_cast<uint8_t>((id >> 8) & 0xff));
}
};  // namespace utils
};  // namespace sensors
