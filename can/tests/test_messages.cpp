#include "catch2/catch.hpp"
#include "can/core/messages.hpp"

using namespace can_messages;

SCENARIO("message building works") {
    GIVEN("a get status response body") {
        auto arr = std::array<uint8_t, 5>{1, 2, 3, 4, 5};
        WHEN("parsed") {
            auto r = GetStatusResponse::parse(arr);
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.data == 0x02030405);
                REQUIRE(r.status == 0x01);
            }
        }
    }

    GIVEN("a set speed request body") {
        auto arr = std::array<uint8_t, 4>{1, 2, 3, 4};
        WHEN("parsed") {
            auto r = SetSpeedRequest::parse(arr);
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.mm_sec == 0x01020304);
            }
        }
    }

    GIVEN("a get speed response body") {
        auto arr = std::array<uint8_t, 4>{1, 2, 3, 4};
        WHEN("parsed") {
            auto r = GetSpeedResponse::parse(arr);
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.mm_sec == 0x01020304);
            }
        }
    }
}