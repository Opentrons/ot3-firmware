#pragma once

#include <concepts>

#include "ids.hpp"

namespace message_core {

using namespace can_ids;

/**
 * Maximum bytes in can message payload
 */
auto constexpr MaxMessageSize = 64;

/**
 * Concept describing an item that has a message id
 * @tparam T
 */
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

}  // namespace message_core