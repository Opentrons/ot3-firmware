#pragma once

#include <concepts>
#include <span>
#include <variant>

#include "messages.hpp"

using namespace can_messages;

namespace can_parse {

/**
 * The concept describing the interface of a parsable can message
 * @tparam T
 */
template <typename T>
concept Parsable = requires(std::span<uint8_t>& input) {
    /**
     * It has a static parse factory method.
     */
    { T::parse(input) }
    ->std::same_as<T>;
    /**
     * Has a static public id member.
     */
    { T::id }
    ->std::convertible_to<MessageId>;
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
    Result parse(MessageId message_id, const std::span<uint8_t>& payload) {
        auto result = Result{std::monostate{}};
        // Fold expression over Parsable template type.
        // Create a lambda that accepts the id and body.
        ((([&result, message_id, &payload]() -> void {
             // If message id matches Parsable when will parse the result.
             if (message_id == T::id) {
                 result = T::parse(payload);
             }
         })()),
         ...);
        return result;
    }
};

}  // namespace can_parse
