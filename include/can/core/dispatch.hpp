#pragma once

#include "can_buffer_task.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/message_buffer.hpp"
#include "parse.hpp"

using namespace can_parse;
using namespace can_message_buffer;

namespace can_dispatch {

/**
 * A message id lookup.
 * @tparam T Types that match the HasMessageID concept.
 */
template <HasMessageID... T>
class MessageIdFilter {
  public:
    MessageIdFilter() {
        // Use fold to populate array
        auto i = 0;
        ((([&i, this]() -> void {
             this->a[i++] = static_cast<uint32_t>(T::id);
         })()),
         ...);
        // Sort for binary search
        std::sort(a.begin(), a.end());
    }

    /**
     * Look up message id in collection
     * @param id message id
     * @return True if present
     */
    [[nodiscard]] auto in(uint32_t id) const -> bool {
        return std::binary_search(a.cbegin(), a.cend(), id);
    }

  private:
    std::array<uint32_t, sizeof...(T)> a{};
};

/**
 * Concept describing a message handling type.
 * @tparam T The type of the class
 * @tparam Message The message types it handles
 */
template <typename T, typename... Message>
concept HandlesMessages =
    requires(T t, std::variant<std::monostate, Message...>& message) {
    // Has a handle method that accepts a variant.
    {t.handle(message)};
};

/**
 * A CanMessageBufferListener that parses messages and notifies a Handler
 * @tparam HandlerType A HandlesMessages type
 * @tparam MessageTypes The Message types this handles
 */
template <typename HandlerType, CanMessage... MessageTypes>
requires HandlesMessages<HandlerType, MessageTypes...>
class DispatchParseTarget {
  public:
    DispatchParseTarget(HandlerType& handler) : handler{handler}, parser{} {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        auto result =
            parser.parse(MessageId{(uint16_t)arbitration_id}, input, limit);
        handler.handle(result);
    }

  private:
    HandlerType& handler;
    Parser<MessageTypes...> parser;
};

/**
 * A CanMessageBufferListener that will write messages to a MessageBuffer only
 * if arbitration id matches message ids in MessageTypes
 *
 * @tparam BufferType a MessageBuffer
 * @tparam MessageTypes The message types
 */
template <message_buffer::MessageBuffer BufferType,
          HasMessageID... MessageTypes>
class DispatchBufferTarget {
  public:
    explicit DispatchBufferTarget(BufferType& buffer) : writer{buffer}, coll{} {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        // TODO pick out the message id
        if (coll.in(arbitration_id)) {
            writer.send(arbitration_id, input, limit, 100);
        }
    }

  private:
    CanMessageBufferWriter<BufferType> writer;
    MessageIdFilter<MessageTypes...> coll;
};

/**
 * A CanMessageBufferListener that will dispatch messages to other
 * CanMessageBufferListeners
 * @tparam Listener CanMessageBufferListener objects
 */
template <CanMessageBufferListener... Listener>
class Dispatcher {
  public:
    explicit Dispatcher(Listener*... listener) : registered{listener...} {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        std::apply(
            [arbitration_id, input, limit](auto... x) {
                (x->handle(arbitration_id, input, limit), ...);
            },
            registered);
    }

  private:
    std::tuple<Listener*...> registered;
};

}  // namespace can_dispatch