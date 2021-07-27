#include "can/core/messages.hpp"
#include "catch2/catch.hpp"

using namespace can_messages;

SCENARIO("message deserializing works") {
    GIVEN("a get status response body") {
        auto arr = std::array<uint8_t, 5>{1, 2, 3, 4, 5};
        WHEN("constructed") {
            auto r = GetStatusResponse::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.data == 0x02030405);
                REQUIRE(r.status == 0x01);
            }
        }
    }

    GIVEN("a set speed request body") {
        auto arr = std::array<uint8_t, 4>{1, 2, 3, 4};
        WHEN("constructed") {
            auto r = SetSpeedRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.mm_sec == 0x01020304);
            }
        }
    }

    GIVEN("a get speed response body") {
        auto arr = std::array<uint8_t, 4>{1, 2, 3, 4};
        WHEN("constructed") {
            auto r = GetSpeedResponse::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.mm_sec == 0x01020304);
            }
        }
    }
}

SCENARIO("message serializing works") {
    GIVEN("a get status response message") {
        auto message = GetStatusResponse{1, 2};
        auto arr = std::array<uint8_t, 5>{0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] == 1);
                REQUIRE(body.data()[1] == 0);
                REQUIRE(body.data()[2] == 0);
                REQUIRE(body.data()[3] == 0);
                REQUIRE(body.data()[4] == 2);
            }
            THEN("size must be returned") { REQUIRE(size == 5); }
        }
    }

    GIVEN("a set speed request message") {
        auto message = SetSpeedRequest{0x10023300};
        auto arr = std::array<uint8_t, 4>{0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] == 0x10);
                REQUIRE(body.data()[1] == 0x02);
                REQUIRE(body.data()[2] == 0x33);
                REQUIRE(body.data()[3] == 0);
            }
            THEN("size must be returned") { REQUIRE(size == 4); }
        }
    }

    GIVEN("a get speed response message") {
        auto message = GetSpeedResponse{0x12344321};
        auto arr = std::array<uint8_t, 4>{0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] == 0x12);
                REQUIRE(body.data()[1] == 0x34);
                REQUIRE(body.data()[2] == 0x43);
                REQUIRE(body.data()[3] == 0x21);
            }
            THEN("size must be returned") { REQUIRE(size == 4); }
        }
    }
}