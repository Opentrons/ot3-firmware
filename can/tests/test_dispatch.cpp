#include "can/core/dispatch.hpp"
#include "can/core/messages.hpp"
#include "catch2/catch.hpp"

using namespace can_dispatch;
using namespace can_messages;

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

    struct Buffer {
        auto send(const uint8_t* buffer, std::size_t buffer_length,
                  uint32_t timeout) -> std::size_t {
            for (std::size_t i = 0; i < buffer_length; i++) {
                result[i] = *(buffer + i);
            }
            size = buffer_length;
            return buffer_length;
        }
        auto send_from_isr(const uint8_t* buffer, std::size_t buffer_length)
            -> std::size_t {
            return 0;
        }
        auto receive(uint8_t* buffer, std::size_t buffer_length,
                     uint32_t timeout) -> std::size_t {
            return 0;
        }

        std::array<uint8_t, 68> result;
        std::size_t size = 0;
    };

    GIVEN(
        "A DispatchBufferTarget that accepts HeartbeatRequest and "
        "HeartbeatResponse") {
        auto l = Buffer{};
        auto buff = BufferType{0xaa};
        auto subject =
            DispatchBufferTarget<Buffer, HeartbeatRequest, HeartbeatResponse>(l);

        WHEN("Given a HeartbeatRequest") {
            subject.handle(static_cast<uint32_t>(HeartbeatRequest::id),
                           buff.begin(), buff.end());
            THEN("send is called.") {
                REQUIRE(l.size == 5);
                REQUIRE(l.result[0] == 0);
                REQUIRE(l.result[1] == 0);
                REQUIRE(l.result[2] == 0xF0);
                REQUIRE(l.result[3] == 0);
                REQUIRE(l.result[4] == 0xaa);
            }
        }
    }

    GIVEN(
        "A DispatchTarget that accepts HeartbeatRequest and "
        "HeartbeatResponse") {
        auto l = Buffer{};
        auto buff = BufferType{0x55};
        auto subject =
            DispatchBufferTarget<Buffer, HeartbeatRequest, HeartbeatResponse>(l);

        WHEN("Given a HeartbeatResponse") {
            subject.handle(static_cast<uint32_t>(HeartbeatResponse::id),
                           buff.begin(), buff.end());
            THEN("send is called") {
                REQUIRE(l.size == 5);
                REQUIRE(l.result[0] == 0);
                REQUIRE(l.result[1] == 0);
                REQUIRE(l.result[2] == 0xF0);
                REQUIRE(l.result[3] == 1);
                REQUIRE(l.result[4] == 0x55);
            }
        }
    }

    GIVEN(
        "A DispatchTarget that accepts HeartbeatRequest and "
        "HeartbeatResponse") {
        auto l = Buffer{};
        auto buff = BufferType{};
        auto subject =
            DispatchBufferTarget<Buffer, HeartbeatRequest, HeartbeatResponse>(l);

        WHEN("Given a GetSpeedResponse") {
            subject.handle(static_cast<uint32_t>(GetSpeedResponse::id),
                           buff.begin(), buff.end());
            THEN("listeners are not called") { REQUIRE(l.size == 0); }
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
            subject.handle(static_cast<uint32_t>(HeartbeatRequest::id),
                           buff.begin(), buff.end());
            THEN("handler is called with a parsed HeartbeatRequest object.") {
                REQUIRE(
                    std::holds_alternative<HeartbeatRequest>(l.parsed_message));
            }
        }
    }
}