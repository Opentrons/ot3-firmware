#pragma once

#include <array>
#include <concepts>
#include <variant>

#include "common/core/bit_utils.hpp"
#include "ids.hpp"

using namespace can_ids;

namespace can_parse {

template <typename T>
concept HasMessageID = requires {
    /**
     * Has a static public id member. This is the MessageId that the Parsable
     * knows how to parse.
     */
    { T::id } -> std::convertible_to<MessageId>;
};

/**
 * The concept describing the interface of a parsable can message
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
 * The concept describing the interface of a serializable can message.
 * @tparam T
 */
template <typename T>
concept Serializable = requires(T& t, std::array<uint8_t, 0> buffer) {
    /**
     * It has a serialize method which returns number of bytes written.
     */
    { t.serialize(buffer.begin(), buffer.end()) } -> std::same_as<uint8_t>;
};

/**
 * The concept describing a can message.
 * @tparam T
 */
template <typename T>
concept CanMessage = requires(T& t) {
    {HasMessageID<T>};
    {Parsable<T>};
    {Serializable<T>};
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
    auto parse(MessageId message_id, const Iterator& payload,
               const Iterator& limit) -> Result {
        auto result = Result{std::monostate{}};
        // Fold expression over Parsable template type.
        // Create a lambda that accepts the id and body.
        ((([&result, message_id, &payload, &limit]() -> void {
             // If message id matches Parsable's id parse will build the result.
             if (message_id == T::id) {
                 result = T::parse(payload, limit);
             }
         })()),
         ...);
        return result;
    }
};

}  // namespace can_parse
