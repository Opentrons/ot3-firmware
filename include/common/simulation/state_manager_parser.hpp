#pragma once

#include <concepts>
#include <iterator>
#include <optional>
#include <variant>

// Message types
#include "common/simulation/attached_instruments.hpp"
#include "common/simulation/direction.hpp"
#include "common/simulation/message_ids.hpp"
#include "common/simulation/move_message_hw_ids.hpp"
#include "common/simulation/sync_pin_state.hpp"

namespace state_manager_parser {

using RT = std::variant<std::monostate, SyncPinState,
                        attached_instruments::AttachedInstruments>;

/**
 * @brief Parse the BODY of a sync pin state response.
 *
 * @param begin First byte AFTER the message type
 * @param end Message limit
 */
template <typename Input, typename Limit>
requires std::forward_iterator<Input> && std::sized_sentinel_for<Limit, Input>
auto parse_sync_pin_state_response(Input begin, Limit &) -> SyncPinState {
    char msg_value = *begin;
    return msg_value == '1' ? SyncPinState::HIGH : SyncPinState::LOW;
}

/**
 * @brief Parse the BODY of an attached instruments response.
 *
 * @param begin First byte AFTER the message type
 * @param end Message limit
 */
template <typename Input, typename Limit>
requires std::forward_iterator<Input> && std::sized_sentinel_for<Limit, Input>
auto parse_attached_instruments_response(Input begin, Limit &end)
    -> attached_instruments::AttachedInstruments {
    auto parse_next = [&begin, &end] {
        if (begin == end) {
            return false;
        }
        char msg_value = *begin;
        std::advance(begin, 1);
        return msg_value != 0;
    };
    return attached_instruments::AttachedInstruments{
        .pipette_left = parse_next(),
        .pipette_right = parse_next(),
        .gripper = parse_next(),
    };
}

template <typename Input, typename Limit>
requires std::forward_iterator<Input> && std::sized_sentinel_for<Limit, Input>
auto parse_state_manager_response(Input begin, Limit &end) -> RT {
    if (begin == end) {
        return RT();
    }
    uint8_t msg_type = *begin;
    std::advance(begin, 1);
    if (begin == end) {
        return RT();
    }
    if (msg_type == static_cast<uint8_t>(MessageID::get_sync_pin_state)) {
        return RT(parse_sync_pin_state_response(begin, end));
    }
    if (msg_type == 4) {
        return RT(parse_attached_instruments_response(begin, end));
    }
    return RT();
}

};  // namespace state_manager_parser