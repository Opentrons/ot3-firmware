#include <array>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "can/core/parse.hpp"
#include "catch2/catch.hpp"

using namespace can::ids;
using namespace can::messages;
using namespace can::parse;

SCENARIO("can parse works") {
    auto parser = Parser<HeartbeatRequest>{};
    GIVEN("a heartbeat request id and body") {
        auto arr = std::array<uint8_t, 0>{};
        auto message_id = MessageId::heartbeat_request;
        WHEN("parsed") {
            auto r = parser.parse(message_id, arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(std::holds_alternative<HeartbeatRequest>(r));
            }
        }
    }
    GIVEN("an unsupported message id and body") {
        auto arr = std::array<uint8_t, 0>{};
        auto message_id = MessageId::heartbeat_response;
        WHEN("parsed") {
            auto r = parser.parse(message_id, arr.begin(), arr.end());
            THEN("it returns an uninitialized result") {
                REQUIRE(std::holds_alternative<std::monostate>(r));
            }
        }
    }
}
