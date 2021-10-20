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
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.data = 24;
                REQUIRE(other != r);
            }
        }
    }

    GIVEN("a move request body") {
        auto arr = std::array<uint8_t, 8>{1, 2, 3, 4, 5, 6, 7, 8};
        WHEN("constructed") {
            auto r = MoveRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.target_position == 0x01020304);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.target_position = 10;
                REQUIRE(other != r);
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
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.mm_sec = 1;
                REQUIRE(other != r);
            }
        }
    }

    GIVEN("a device info response body") {
        auto arr = std::array<uint8_t, 5>{0x20, 2, 3, 4, 5};
        WHEN("constructed") {
            auto r = DeviceInfoResponse::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.node_id == NodeId::pipette);
                REQUIRE(r.version == 0x02030405);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.version = 125;
                REQUIRE(other != r);
            }
        }
    }
}

SCENARIO("message serializing works") {
    GIVEN("a get status response message") {
        auto message = GetStatusResponse{{}, 1, 2};
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

    GIVEN("a set steps request message") {
        auto message = MoveRequest{{}, 0x10023300};
        auto arr = std::array<uint8_t, 8>{};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] == 0x10);
                REQUIRE(body.data()[1] == 0x02);
                REQUIRE(body.data()[2] == 0x33);
                REQUIRE(body.data()[3] == 0x00);
            }
            THEN("size must be returned") { REQUIRE(size == 4); }
        }
    }

    GIVEN("a get speed response message") {
        auto message = GetSpeedResponse{{}, 0x12344321};
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

    GIVEN("a device info response message") {
        auto message = DeviceInfoResponse{{}, NodeId::pipette, 0x00220033};
        auto arr = std::array<uint8_t, 5>{0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] == 0x20);
                REQUIRE(body.data()[1] == 0x00);
                REQUIRE(body.data()[2] == 0x22);
                REQUIRE(body.data()[3] == 0x00);
                REQUIRE(body.data()[4] == 0x33);
            }
            THEN("size must be returned") { REQUIRE(size == 5); }
        }
    }

    GIVEN("a get speed request message") {
        auto message = GetSpeedRequest{{}};
        auto arr = std::array<uint8_t, 4>{0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("size must be returned") { REQUIRE(size == 0); }
        }
    }
}
