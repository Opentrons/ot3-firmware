#pragma once

#include <functional>

#include "arbitration_id.hpp"
#include "can_message_buffer.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/message_buffer.hpp"
#include "ids.hpp"
#include "parse.hpp"

namespace can_dispatch {

using namespace can_parse;
using namespace can_message_buffer;
using namespace can_arbitration_id;

/**
 * A message id lookup.
 * @tparam T Types that match the HasMessageID concept.
 */
template <HasMessageID... T>
class MessageIdCollection {
  public:
    MessageIdCollection() {
        // Use fold to populate array
        auto i = 0;
        ((([&i, this]() -> void { this->arr[i++] = T::id; })()), ...);
        // Sort for binary search
        std::sort(arr.begin(), arr.end());
    }

    /**
     * Look up message id in collection
     * @param id message id
     * @return True if present
     */
    [[nodiscard]] auto in(can_ids::MessageId id) const -> bool {
        return std::binary_search(arr.cbegin(), arr.cend(), id);
    }

  private:
    std::array<can_ids::MessageId, sizeof...(T)> arr{};
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
requires HandlesMessages<HandlerType, MessageTypes...> &&
    (!std::movable<HandlerType> &&
     !std::copyable<HandlerType>)class DispatchParseTarget {
  public:
    DispatchParseTarget(HandlerType& handler) : handler{handler}, parser{} {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        auto arb = ArbitrationId{.id = arbitration_id};
        auto result =
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
            parser.parse(MessageId{static_cast<uint16_t>(arb.parts.message_id)},
                         input, limit);
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
requires(!std::movable<BufferType> &&
         !std::copyable<BufferType>) class DispatchBufferTarget {
  public:
    explicit DispatchBufferTarget(BufferType& buffer)
        : writer{buffer}, coll{} {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        auto arb = ArbitrationId{.id = arbitration_id};
        if (coll.in(can_ids::MessageId{
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                static_cast<uint16_t>(arb.parts.message_id)})) {
            writer.send(arbitration_id, input, limit, 100);
        }
    }

  private:
    CanMessageBufferWriter<BufferType> writer;
    MessageIdCollection<MessageTypes...> coll;
};

/**
 * A CanMessageBufferListener that will dispatch messages to other
 * CanMessageBufferListeners
 * @tparam Listener CanMessageBufferListener objects
 */
template <CanMessageBufferListener... Listener>
class Dispatcher {
  public:
    using ArbitrationIdTest = std::function<bool(uint32_t identifier)>;
    explicit Dispatcher(ArbitrationIdTest test, Listener&... listener)
        : registered{listener...}, test{std::move(test)} {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        if (test(arbitration_id)) {
            std::apply(
                [arbitration_id, input, limit](auto&... x) {
                    (x.handle(arbitration_id, input, limit), ...);
                },
                registered);
        }
    }

  private:
    std::tuple<Listener&...> registered;
    ArbitrationIdTest test;
};

}  // namespace can_dispatch
