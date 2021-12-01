#include "can/core/arbitration_id.hpp"
#include "can/core/dispatch.hpp"
#include "can/core/messages.hpp"
#include "can/tests/mock_message_buffer.hpp"
#include "catch2/catch.hpp"
#include "common/core/bit_utils.hpp"

using namespace can_dispatch;
using namespace can_messages;
using namespace can_arbitration_id;
using namespace mock_message_buffer;

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
