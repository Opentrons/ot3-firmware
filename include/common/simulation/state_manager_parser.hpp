#pragma once

#include <concepts>
#include <iterator>
#include <optional>
#include <variant>

// Message types
#include "common/simulation/direction.hpp"
#include "common/simulation/message_ids.hpp"
#include "common/simulation/move_message_hw_ids.hpp"
#include "common/simulation/sync_pin_state.hpp"

namespace state_manager_parser {

using RT = std::variant<std::monostate, SyncPinState>;

template <typename Input, typename Limit>
requires std::forward_iterator<Input> && std::sized_sentinel_for<Limit, Input>
auto parse_state_manager_response(Input begin, Limit &end) -> RT {
    if(begin == end) {
        return RT();
    }
    uint8_t msg_type = *begin;
    std::advance(begin, 1);
    if(begin == end) {
        return RT();
    }
    if(msg_type != static_cast<uint8_t>(MessageID::get_sync_pin_state)) {
        // For now, we only parse sync pin state responses
        return RT();
    }
    // Read the rest of the message as ascii text - 0 or 1
    char msg_value = *begin;
    return RT(
        msg_value == '1' ? SyncPinState::HIGH : SyncPinState::LOW
    );
}

};