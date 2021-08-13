#pragma once

#include <variant>
#include "common/core/bit_utils.hpp"
#include "ids.hpp"
#include "message_core.hpp"

using namespace can_ids;
using namespace message_core;

namespace can_parse {

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
