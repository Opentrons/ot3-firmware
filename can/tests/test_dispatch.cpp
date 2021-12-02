#include "can/core/arbitration_id.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "can/tests/mock_message_buffer.hpp"
#include "catch2/catch.hpp"
#include "common/core/bit_utils.hpp"

using namespace can_dispatch;
using namespace can_messages;
using namespace can_arbitration_id;
using namespace mock_message_buffer;

SCENARIO("Dispatcher") {
    using BufferType = std::array<uint8_t, 1>;
    using BufferIterator = BufferType::iterator;

    struct Listener {
        Listener() : id{0}, iter{nullptr}, limit{nullptr} {}
        Listener(const Listener&) = delete;
        Listener(const Listener&&) = delete;
        Listener& operator=(const Listener&) = delete;
        Listener&& operator=(const Listener&&) = delete;

        void handle(uint32_t id, BufferIterator i, BufferIterator limit) {
            this->id = id;
            this->iter = i;
            this->limit = limit;
        }
        uint32_t id;
        BufferIterator iter;
        BufferIterator limit;
    };
    GIVEN("a dispatcher with two listeners") {
        auto l1 = Listener{};
        auto l2 = Listener{};
        auto buff = BufferType{1};
        uint32_t arb_id = 1234;

        auto subject = Dispatcher([](auto _) -> bool { return true; }, l1, l2);

        WHEN("dispatching a message") {
            subject.handle(arb_id, buff.begin(), buff.end());
            THEN("listeners are called") {
                REQUIRE(l1.id == arb_id);
                REQUIRE(l1.iter == buff.begin());
                REQUIRE(l1.limit == buff.end());
                REQUIRE(l2.id == arb_id);
                REQUIRE(l2.iter == buff.begin());
                REQUIRE(l2.limit == buff.end());
            }
        }
    }

    GIVEN("a dispatcher with two listeners for head_left") {
        auto l1 = Listener{};
        auto l2 = Listener{};
        auto buff = BufferType{1};
        struct CheckForNodeId {
            NodeId node_id;
            auto operator()(uint32_t arbitration_id) const {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                auto arb = ArbitrationId{.id = arbitration_id};
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                auto _node_id = static_cast<uint16_t>(arb.parts.node_id);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                auto tmp = static_cast<uint16_t>(node_id);
                return (_node_id == tmp);
            }
        };

        CheckForNodeId check_for_node_id_left{.node_id = NodeId::head_left};
        auto subject = Dispatcher(check_for_node_id_left, l1, l2);

        WHEN("dispatching a message") {
            auto arbitration_id = ArbitrationId{.id = 0};
            arbitration_id.parts.node_id =
                static_cast<uint16_t>(NodeId::head_left);
            subject.handle(arbitration_id.id, buff.begin(), buff.end());
            THEN("listeners are called") {
                REQUIRE(l1.id == static_cast<uint32_t>(arbitration_id.id));
                REQUIRE(l1.iter == buff.begin());
                REQUIRE(l1.limit == buff.end());
                REQUIRE(l2.id == static_cast<uint32_t>(arbitration_id.id));
                REQUIRE(l2.iter == buff.begin());
                REQUIRE(l2.limit == buff.end());
            }
        }
    }

    GIVEN("a dispatcher with two listeners for head_right ") {
        auto l1 = Listener{};
        auto l2 = Listener{};
        auto buff = BufferType{1};
        struct CheckForNodeId {
            NodeId node_id;
            auto operator()(uint32_t arbitration_id) const {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                auto arb = ArbitrationId{.id = arbitration_id};
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                auto _node_id = static_cast<uint16_t>(arb.parts.node_id);
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
                auto tmp = static_cast<uint16_t>(node_id);
                return (_node_id == tmp);
            }
        };

        CheckForNodeId check_node_id_right{.node_id = NodeId::head_right};
        auto subject = Dispatcher(check_node_id_right, l1, l2);

        WHEN(
            "dispatching a head_left message to a dispatcher expecting "
            "head_right") {
            auto arbitration_id = ArbitrationId{.id = 0};
            arbitration_id.parts.node_id =
                static_cast<uint16_t>(NodeId::head_left);
            subject.handle(arbitration_id.id, buff.begin(), buff.end());
            THEN("listeners are are not called") {
                REQUIRE(l1.id == 0);
                REQUIRE(l1.iter == nullptr);
                REQUIRE(l1.limit == nullptr);
                REQUIRE(l2.id == 0);
                REQUIRE(l2.iter == nullptr);
                REQUIRE(l2.limit == nullptr);
            }
        }
    }
}

SCENARIO("DispatchBufferTarget") {
    using BufferType = std::array<uint8_t, 1>;

    GIVEN(
        "A DispatchBufferTarget that accepts HeartbeatRequest and "
        "HeartbeatResponse") {
        auto l = MockMessageBuffer<1>{};
        auto buff = BufferType{0xaa};
        auto subject = DispatchBufferTarget<decltype(l), HeartbeatRequest,
                                            HeartbeatResponse>(l);

        WHEN("Given a HeartbeatRequest") {
            auto arbitration_id = ArbitrationId{.id = 0};
            arbitration_id.parts.message_id =
                static_cast<uint16_t>(HeartbeatRequest::id);
            subject.handle(arbitration_id.id, buff.begin(), buff.end());
            THEN("send is called.") {
                auto arbitration_id_buffer = std::array<uint8_t, 4>{};
                auto iter = arbitration_id_buffer.begin();
                iter = bit_utils::int_to_bytes(arbitration_id.id, iter,
                                               arbitration_id_buffer.end());
                REQUIRE(l.length == 5);
                REQUIRE(l.buff[0] == arbitration_id_buffer[0]);
                REQUIRE(l.buff[1] == arbitration_id_buffer[1]);
                REQUIRE(l.buff[2] == arbitration_id_buffer[2]);
                REQUIRE(l.buff[3] == arbitration_id_buffer[3]);
                REQUIRE(l.buff[4] == 0xaa);
            }
        }
    }

    GIVEN(
        "A DispatchTarget that accepts HeartbeatRequest and "
        "HeartbeatResponse") {
        auto l = MockMessageBuffer<1>{};
        auto buff = BufferType{0x55};
        auto subject = DispatchBufferTarget<decltype(l), HeartbeatRequest,
                                            HeartbeatResponse>(l);

        WHEN("Given a HeartbeatResponse") {
            auto arbitration_id = ArbitrationId{.id = 0};
            arbitration_id.parts.message_id =
                static_cast<uint16_t>(HeartbeatResponse::id);
            subject.handle(arbitration_id.id, buff.begin(), buff.end());
            THEN("send is called") {
                auto arbitration_id_buffer = std::array<uint8_t, 4>{};
                auto iter = arbitration_id_buffer.begin();
                iter = bit_utils::int_to_bytes(arbitration_id.id, iter,
                                               arbitration_id_buffer.end());
                REQUIRE(l.length == 5);
                REQUIRE(l.buff[0] == arbitration_id_buffer[0]);
                REQUIRE(l.buff[1] == arbitration_id_buffer[1]);
                REQUIRE(l.buff[2] == arbitration_id_buffer[2]);
                REQUIRE(l.buff[3] == arbitration_id_buffer[3]);
                REQUIRE(l.buff[4] == 0x55);
            }
        }
    }

    GIVEN(
        "A DispatchTarget that accepts HeartbeatRequest and "
        "HeartbeatResponse") {
        auto l = MockMessageBuffer<1>{};
        auto buff = BufferType{};
        auto subject = DispatchBufferTarget<decltype(l), HeartbeatRequest,
                                            HeartbeatResponse>(l);

        WHEN("Given a GetMoveGroupResponse") {
            auto arbitration_id = ArbitrationId{.id = 0};
            arbitration_id.parts.message_id =
                static_cast<uint16_t>(GetMoveGroupResponse::id);
            subject.handle(arbitration_id.id, buff.begin(), buff.end());
            THEN("listeners are not called") { REQUIRE(l.length == 0); }
        }
    }
}

SCENARIO("DispatchParseTarget") {
    using BufferType = std::array<uint8_t, 0>;
    using MessageTypes =
        std::variant<std::monostate, HeartbeatRequest, HeartbeatResponse>;

    struct Handler {
        Handler() {}
        Handler(const Handler&) = delete;
        Handler(const Handler&&) = delete;
        Handler& operator=(const Handler&) = delete;
        Handler&& operator=(const Handler&&) = delete;

        void handle(MessageTypes& m) { parsed_message = m; }
        MessageTypes parsed_message{};
    };

    GIVEN(
        "A DispatchParseTarget that accepts hearbeat request and response "
        "types") {
        auto l = Handler{};
        auto buff = BufferType{};
        auto subject =
            DispatchParseTarget<Handler, HeartbeatRequest, HeartbeatResponse>(
                l);

        WHEN("Given a HeartbeatRequest") {
            auto arbitration_id = ArbitrationId{.id = 0};
            arbitration_id.parts.message_id =
                static_cast<uint16_t>(HeartbeatRequest::id);
            subject.handle(arbitration_id.id, buff.begin(), buff.end());
            THEN("handler is called with a parsed HeartbeatRequest object.") {
                REQUIRE(
                    std::holds_alternative<HeartbeatRequest>(l.parsed_message));
            }
        }
    }
}
