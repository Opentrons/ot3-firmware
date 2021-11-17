#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "catch2/catch.hpp"

using namespace can_messages;

SCENARIO("message deserializing works") {
    GIVEN("a move request body") {
        auto arr =
            std::array<uint8_t, 12>{1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc};
        WHEN("constructed") {
            auto r = MoveRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.duration == 0x01020304);
                REQUIRE(r.velocity == 0x05060708);
                REQUIRE(r.acceleration == 0x090a0b0c);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.duration = 10;
                REQUIRE(other != r);
            }
        }
    }

    GIVEN("a set steps request message") {
        auto arr = std::array<uint8_t, 12>{0x12, 0x34, 0x56, 0x78, 0xaa, 0xbb,
                                           0xcc, 0xdd, 0xee, 0xff, 0x11, 0x00};
        WHEN("constructed") {
            auto r = MoveRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.duration == 0x12345678);
                REQUIRE(r.velocity == static_cast<int32_t>(0xaabbccdd));
                REQUIRE(r.acceleration == static_cast<int32_t>(0xeeff1100));
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.duration = 125;
                REQUIRE(other != r);
            }
        }
    }
}

SCENARIO("message serializing works") {
    GIVEN("a get status response message") {
        auto message = GetStatusResponse{.status = 1, .data = 2};
        message.set_node_id(can_ids::NodeId::pipette);
        auto arr = std::array<uint8_t, 6>{0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] ==
                        static_cast<uint8_t>(can_ids::NodeId::pipette));
                REQUIRE(body.data()[1] == 1);
                REQUIRE(body.data()[2] == 0);
                REQUIRE(body.data()[3] == 0);
                REQUIRE(body.data()[4] == 0);
                REQUIRE(body.data()[5] == 2);
            }
            THEN("size must be returned") { REQUIRE(size == 6); }
        }
    }

    GIVEN("a MoveGroupCompleted message") {
        auto message = MoveGroupCompleted{.group_id = 1};
        message.set_node_id(can_ids::NodeId::pipette);
        auto arr = std::array<uint8_t, 2>{};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] ==
                        static_cast<uint8_t>(can_ids::NodeId::pipette));
                REQUIRE(body.data()[1] == 1);
            }
            THEN("size must be returned") { REQUIRE(size == 2); }
        }
    }

    GIVEN("a device info response message") {
        auto message = DeviceInfoResponse{.version = 0x00220033};
        message.set_node_id(can_ids::NodeId::pipette);
        auto arr = std::array<uint8_t, 5>{0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] ==
                        static_cast<uint8_t>(can_ids::NodeId::pipette));
                REQUIRE(body.data()[1] == 0x00);
                REQUIRE(body.data()[2] == 0x22);
                REQUIRE(body.data()[3] == 0x00);
                REQUIRE(body.data()[4] == 0x33);
            }
            THEN("size must be returned") { REQUIRE(size == 5); }
        }
    }
}
