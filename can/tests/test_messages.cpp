#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "catch2/catch.hpp"

using namespace can_messages;

SCENARIO("message deserializing works") {
    GIVEN("a set motion constraints request body") {
        auto arr = std::array<uint8_t, 16>{
            1, 2, 3, 4, 5, 6, 7, 8, 9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x11};
        WHEN("constructed") {
            auto r = SetMotionConstraints::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.min_velocity == 0x01020304);
                REQUIRE(r.max_velocity == 0x05060708);
                REQUIRE(r.min_acceleration == 0x090a0b0c);
                REQUIRE(r.max_acceleration == 0x0d0e0f11);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.min_velocity = 10;
                REQUIRE(other != r);
            }
        }
    }

    GIVEN("a read motor driver register message") {
        auto arr = std::array<uint8_t, 1>{0x12};
        WHEN("constructed") {
            auto r = ReadMotorDriverRegister::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.reg_address == 0x12);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.reg_address = 125;
                REQUIRE(other != r);
            }
        }
    }
}

SCENARIO("message serializing works") {
    GIVEN("a MoveCompleted message") {
        auto message = MoveCompleted{.group_id = 1,
                                     .seq_id = 2,
                                     .current_position = 0x3456789a,
                                     .ack_id = 1};
        message.set_node_id(can_ids::NodeId::pipette);
        auto arr = std::array<uint8_t, 8>{};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] ==
                        static_cast<uint8_t>(can_ids::NodeId::pipette));
                REQUIRE(body.data()[1] == 1);
                REQUIRE(body.data()[2] == 2);
                REQUIRE(body.data()[3] == 0x34);
                REQUIRE(body.data()[4] == 0x56);
                REQUIRE(body.data()[5] == 0x78);
                REQUIRE(body.data()[6] == 0x9a);
                REQUIRE(body.data()[7] == 1);
            }
            THEN("size must be returned") { REQUIRE(size == 8); }
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
