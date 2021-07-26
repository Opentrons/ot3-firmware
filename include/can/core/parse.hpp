#pragma once

#include <concepts>
#include <variant>

#include "messages.hpp"

using namespace can_messages;

namespace can_parse {

/**
 * The concept describing the interface of a parsable can message
 * @tparam T
 */
template <typename T>
concept Parsable = requires(BodyType& input) {
    /**
     * It has a static parse factory method.
     */
    { T::parse(input) } -> std::same_as<T>;
    /**
     * Has a static public id member. This is the MessageId that the Parsable
     * knows how to parse.
     */
    { T::id } -> std::convertible_to<MessageId>;
};

/**
 * The concept describing the interface of a serializable can message.
 * @tparam T
 */
template <typename T>
concept Serializable = requires(const T& t, BodyType::iterator out) {
    /**
     * It has a serialize method
     */
    { t.serialize(out) } -> std::same_as<BodyType::iterator>;
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
    auto parse(MessageId message_id, const BodyType& payload) -> Result {
        auto result = Result{std::monostate{}};
        // Fold expression over Parsable template type.
        // Create a lambda that accepts the id and body.
        ((([&result, message_id, &payload]() -> void {
             // If message id matches Parsable's id parse will build the result.
             if (message_id == T::id) {
                 result = T::parse(payload);
             }
         })()),
         ...);
        return result;
    }
};

}  // namespace can_parse
