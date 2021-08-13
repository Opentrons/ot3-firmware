#include "can/core/dispatch.hpp"
#include "can/core/messages.hpp"
#include "can/tests/mock_message_buffer.hpp"
#include "catch2/catch.hpp"
#include "common/core/bit_utils.hpp"

using namespace can_dispatch;
using namespace can_messages;
using namespace mock_message_buffer;

SCENARIO("Dispatcher") {
    using BufferType = std::array<uint8_t, 1>;
    using BufferIterator = BufferType::iterator;

    struct Listener {
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
        auto subject = Dispatcher(&l1, &l2);

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

        WHEN("Given a GetSpeedResponse") {
            auto arbitration_id = ArbitrationId{.id = 0};
            arbitration_id.parts.message_id =
                static_cast<uint16_t>(GetSpeedResponse::id);
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