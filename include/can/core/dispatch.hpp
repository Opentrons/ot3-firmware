#pragma once

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
 * A CanMessageBufferListener for specific nodes that parses messages and
 * notifies a Handler
 * @tparam HandlerType A HandlesMessages type
 * @tparam MessageTypes The Message types this handles
 */
template <typename HandlerType, CanMessage... MessageTypes>
requires HandlesMessages<HandlerType, MessageTypes...> &&
    (!std::movable<HandlerType> &&
     !std::copyable<HandlerType>)class DispatchParseTargetNode {
  public:
    DispatchParseTargetNode(HandlerType& handler, uint16_t node_id)
        : handler{handler}, parser{}, node(node_id) {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        if (check_motor(arbitration_id)) {
            auto arb = ArbitrationId{.id = arbitration_id};
            auto result = parser.parse(
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                MessageId{static_cast<uint16_t>(arb.parts.message_id)}, input,
                limit);
            handler.handle(result);
        }
    }

  private:
    HandlerType& handler;
    Parser<MessageTypes...> parser;
    uint16_t node;
    auto check_motor(uint32_t arbitration_id) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        auto arb = ArbitrationId{.id = arbitration_id};
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
        auto _node_id = static_cast<uint16_t>(arb.parts.node_id);
        return (_node_id == this->node);
    }
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
    explicit Dispatcher(Listener&... listener) : registered{listener...} {}

    template <bit_utils::ByteIterator Input, typename Limit>
    requires std::sentinel_for<Limit, Input>
    void handle(uint32_t arbitration_id, Input input, Limit limit) {
        std::apply(
            [arbitration_id, input, limit](auto&... x) {
                (x.handle(arbitration_id, input, limit), ...);
            },
            registered);
    }

  private:
    std::tuple<Listener&...> registered;
};

}  // namespace can_dispatch
