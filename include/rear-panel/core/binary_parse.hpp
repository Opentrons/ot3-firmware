#pragma once

#include <variant>

#include "common/core/bit_utils.hpp"
#include "rear-panel/core/bin_msg_ids.hpp"
#include "rear-panel/core/messages.hpp"

namespace binary_parse {

/**
 * The concept describing the interface of a parsable message
 * @tparam T
 */
template <class T>
concept Parsable = requires(std::array<uint8_t, 0> buffer) {
    /**
     * It has a static parse factory method.
     */
    { T::parse(buffer.begin(), buffer.end()) } -> std::same_as<T>;
    { T::parse(buffer.cbegin(), buffer.cend()) } -> std::same_as<T>;
};

/**
 * Parser of can message bodies.
 *
 * @tparam T The types of Parsables (messages) this parser will support.
 */
template <Parsable... T>
class Parser {
  public:
    using Result = std::variant<std::monostate, T...>;

    /**
     * Parse a message body and return a Result type.
     *
     * @param message_id The message id
     * @param payload The body
     * @return A Result variant
     */
    template <bit_utils::ByteIterator Iterator>
    auto parse(messages::MessageType message_type, const Iterator& payload,
               const Iterator& limit) -> Result {
        auto result = Result{std::monostate{}};
        // Fold expression over Parsable template type.
        // Create a lambda that accepts the type and body.
        ((([&result, message_type, &payload, &limit]() -> void {
             // If message type matches Parsable's type parse will build the result.
             if (message_type == T::message_type) {
                 result = T::parse(payload, limit);
             }
         })()),
         ...);
        return result;
    }
};

/**
 * Concept describing an item that has a message index
 * @tparam T
 */
template <typename T>
concept HasMessageType = requires {
    /**
     * Has a static public id member. This is the MessageType that the Parsable
     * knows how to parse.
     */
    { T::message_type } -> std::convertible_to<uint16_t>;
};

}  // namespace binary_parse
