#include <span>
#include <array>
#include "common/core/can/messages.hpp"
#include "common/core/can/ids.hpp"
#include "common/core/can/parse.hpp"
#include "catch2/catch.hpp"

using namespace can_ids;
using namespace can_messages;
using namespace can_parse;


SCENARIO("can parse works") {
    auto parser = Parser < HeartbeatRequest > {};
    GIVEN("a heartbeat request id and body") {
        auto arr = std::array < uint8_t, 0>{ };
        auto sp = std::span{arr};
        auto message_id = MessageId::heartbeat_request;
        WHEN("parsed") {
            auto r = parser.parse(message_id, sp);
            THEN("it is converted to a the correct structure") {
                REQUIRE(std::holds_alternative<HeartbeatRequest>(r));
            }
        }
    }
    GIVEN("an unsupported message id and body") {
        auto arr = std::array < uint8_t, 0>{ };
        auto sp = std::span{arr};
        auto message_id = MessageId::heartbeat_response;
        WHEN("parsed") {
            auto r = parser.parse(message_id, sp);
            THEN("it returns an uninitialized result") {
                REQUIRE(std::holds_alternative<std::monostate>(r));
            }
        }
    }
}
