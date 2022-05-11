#include <cstring>

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

    GIVEN("a write to eeprom message") {
        auto arr = std::array<uint8_t, 9>{// Address
                                          0x12,
                                          // Data Length
                                          7,
                                          // Data
                                          1, 2, 3, 4, 5, 6, 7};
        WHEN("constructed") {
            auto r = WriteToEEPromRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.address == 0x12);
                REQUIRE(r.data_length == 7);
                REQUIRE(r.data[0] == 1);
                REQUIRE(r.data[1] == 2);
                REQUIRE(r.data[2] == 3);
                REQUIRE(r.data[3] == 4);
                REQUIRE(r.data[4] == 5);
                REQUIRE(r.data[5] == 6);
                REQUIRE(r.data[6] == 7);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.address = 125;
                REQUIRE(other != r);
            }
        }
    }

    GIVEN("a write to eeprom message with too large data length") {
        auto arr = std::array<uint8_t, 35>{// Address
                                           0x5,
                                           // Data Length
                                           122,
                                           // Data
                                           0, 1};
        WHEN("constructed") {
            auto r = WriteToEEPromRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.address == 0x05);
                REQUIRE(r.data_length == 8);
                REQUIRE(r.data[0] == 0);
                REQUIRE(r.data[1] == 1);
                REQUIRE(r.data[2] == 0);
                REQUIRE(r.data[3] == 0);
                REQUIRE(r.data[4] == 0);
                REQUIRE(r.data[5] == 0);
                REQUIRE(r.data[6] == 0);
            }
        }
    }
}

SCENARIO("message serializing works") {
    GIVEN("a MoveCompleted message") {
        auto message = MoveCompleted{.group_id = 1,
                                     .seq_id = 2,
                                     .current_position_um = 0x3456789a,
                                     .encoder_position = 0x05803931,
                                     .ack_id = 1};
        auto arr = std::array<uint8_t, 12>{};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] == 1);
                REQUIRE(body.data()[1] == 2);
                REQUIRE(body.data()[2] == 0x34);
                REQUIRE(body.data()[3] == 0x56);
                REQUIRE(body.data()[4] == 0x78);
                REQUIRE(body.data()[5] == 0x9a);
                REQUIRE(body.data()[6] == 0x05);
                REQUIRE(body.data()[7] == 0x80);
                REQUIRE(body.data()[8] == 0x39);
                REQUIRE(body.data()[9] == 0x31);
                REQUIRE(body.data()[10] == 1);
            }
            THEN("size must be returned") { REQUIRE(size == 11); }
        }
    }

    GIVEN("a device info response message") {
        auto message = DeviceInfoResponse{
            .version = 0x00220033, .flags = 0x11445566, .shortsha{"abcdef0"}};
        auto arr = std::array<uint8_t, 17>{0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized into a buffer too small for its values") {
            auto size = message.serialize(arr.begin(), arr.begin() + 10);
            THEN("it is written into the buffer to its end") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x22);
                REQUIRE(body.data()[2] == 0x00);
                REQUIRE(body.data()[3] == 0x33);
                REQUIRE(body.data()[4] == 0x11);
                REQUIRE(body.data()[5] == 0x44);
                REQUIRE(body.data()[6] == 0x55);
                REQUIRE(body.data()[7] == 0x66);
                REQUIRE(body.data()[8] == 'a');
                REQUIRE(body.data()[9] == 'b');
            }
            THEN("it does not write past the end of the buffer") {
                for (uint32_t i = 10; i < arr.size(); i++) {
                    REQUIRE(body.data()[i] == 0);
                }
            }
            THEN("size must be returned") { REQUIRE(size == 10); }
        }
        WHEN("serialized into a buffer larger than needed") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is fully written into the buffer") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x22);
                REQUIRE(body.data()[2] == 0x00);
                REQUIRE(body.data()[3] == 0x33);
                REQUIRE(body.data()[4] == 0x11);
                REQUIRE(body.data()[5] == 0x44);
                REQUIRE(body.data()[6] == 0x55);
                REQUIRE(body.data()[7] == 0x66);
                REQUIRE(body.data()[8] == 'a');
                REQUIRE(body.data()[9] == 'b');

                REQUIRE(body.data()[10] == 'c');
                REQUIRE(body.data()[11] == 'd');
                REQUIRE(body.data()[12] == 'e');
                REQUIRE(body.data()[13] == 'f');
                REQUIRE(body.data()[14] == '0');
                REQUIRE(body.data()[15] == '\0');
            }
            THEN("it does not write past the end of the buffer") {
                REQUIRE(body.data()[16] == 0);
            }
            THEN("size must be returned") { REQUIRE(size == 16); }
        }
    }

    GIVEN("a read from eeprom response") {
        auto data = std::array<uint8_t, 5>{0, 1, 2, 3, 4};
        auto message =
            ReadFromEEPromResponse::create(13, data.cbegin(), data.cend());

        THEN("the length is correctly set.") {
            REQUIRE(message.data_length == data.size());
        }

        WHEN("serialized into too small a buffer") {
            auto arr = std::array<uint8_t, 5>{};
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer.") {
                REQUIRE(arr[0] == 13);
                REQUIRE(arr[1] == 0x5);
                REQUIRE(arr[2] == 0x0);
                REQUIRE(arr[3] == 0x1);
                REQUIRE(arr[4] == 0x2);
            }
            THEN("size is correct") { REQUIRE(size == 5); }
        }

        WHEN("serialized into too large a buffer") {
            auto arr = std::array<uint8_t, ReadFromEEPromResponse::SIZE + 5>{};
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer.") {
                REQUIRE(arr[0] == 13);
                REQUIRE(arr[1] == 0x5);
                REQUIRE(arr[2] == 0x0);
                REQUIRE(arr[3] == 0x1);
                REQUIRE(arr[4] == 0x2);
                REQUIRE(arr[5] == 0x3);
                REQUIRE(arr[6] == 0x4);
                REQUIRE(arr[7] == 0x0);
                REQUIRE(arr[8] == 0x0);
                REQUIRE(arr[9] == 0x0);
            }
            THEN("size is correct") {
                REQUIRE(size == ReadFromEEPromResponse::SIZE);
            }
        }
    }
}
