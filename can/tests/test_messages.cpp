#include <cstring>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/version.h"

using namespace can::messages;

SCENARIO("message deserializing works") {
    GIVEN("a set motion constraints request body") {
        auto arr = std::array<uint8_t, 20>{
            0xde, 0xad, 0xbe, 0xef, 1,   2,   3,   4,   5,   6,
            7,    8,    9,    0xa,  0xb, 0xc, 0xd, 0xe, 0xf, 0x11};
        WHEN("constructed") {
            auto r = SetMotionConstraints::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.message_index == 0xdeadbeef);
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
        auto arr = std::array<uint8_t, 5>{0xde, 0xad, 0xbe, 0xef, 0x12};
        WHEN("constructed") {
            auto r = ReadMotorDriverRegister::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.message_index == 0xdeadbeef);
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

    GIVEN("a gear read motor driver register message") {
        auto arr = std::array<uint8_t, 5>{0xde, 0xad, 0xbe, 0xef, 0x12};
        WHEN("constructed") {
            auto r = GearReadMotorDriverRegister::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.message_index == 0xdeadbeef);
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

    GIVEN("a gear write motor driver register message") {
        auto arr = std::array<uint8_t, 5>{0xde, 0xad, 0xbe, 0xef, 0x12};
        WHEN("constructed") {
            auto r =
                GearWriteMotorDriverRegister::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.message_index == 0xdeadbeef);
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

    GIVEN("a gear write current message") {
        auto arr = std::array<uint8_t, 12>{0xde, 0xad, 0xbe, 0xef, 0x0,  0x12,
                                           0x0,  0x0,  0x0,  0x35, 0x20, 0x0};
        WHEN("constructed") {
            auto r =
                GearWriteMotorCurrentRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.message_index == 0xdeadbeef);
                REQUIRE(r.hold_current == 0x120000);
                REQUIRE(r.run_current == 0x352000);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.hold_current = 125;
                REQUIRE(other != r);
            }
        }
    }

    GIVEN("a write to eeprom message") {
        auto arr =
            std::array<uint8_t, sizeof(eeprom::message::EepromMessage)>{};
        auto iter = arr.begin();
        uint32_t msg_ind = 0xdeadbeef;
        eeprom::types::address addr = 0x12;
        eeprom::types::data_length len = 7;
        // copy message_index to array
        iter = bit_utils::int_to_bytes(msg_ind, iter, iter + sizeof(uint32_t));
        // copy addr to array
        iter = bit_utils::int_to_bytes(addr, iter,
                                       iter + sizeof(eeprom::types::address));
        // copy len to array
        iter = bit_utils::int_to_bytes(
            len, iter, iter + sizeof(eeprom::types::data_length));
        // fill data with 1->7
        for (uint8_t i = 1; i <= len; i++) {
            *(iter++) = i;
        }

        WHEN("constructed") {
            auto r = WriteToEEPromRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.message_index == 0xdeadbeef);
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
        auto arr =
            std::array<uint8_t, sizeof(eeprom::message::EepromMessage)>{};
        auto iter = arr.begin();
        uint32_t msg_ind = 0xdeadbeef;
        eeprom::types::address addr = 0x05;
        eeprom::types::data_length len = 122;
        // copy the message_index
        iter = bit_utils::int_to_bytes(msg_ind, iter, iter + sizeof(uint32_t));
        // copy the address
        iter = bit_utils::int_to_bytes(addr, iter,
                                       iter + sizeof(eeprom::types::address));
        // copy the data_length
        iter = bit_utils::int_to_bytes(
            len, iter, iter + sizeof(eeprom::types::data_length));
        // fill the first two bytes of data
        for (uint8_t i = 0; i < 2; i++) {
            *(iter++) = i;
        }

        WHEN("constructed") {
            auto r = WriteToEEPromRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.message_index == 0xdeadbeef);
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
        auto message = MoveCompleted{.message_index = 0xdeadbeef,
                                     .group_id = 1,
                                     .seq_id = 2,
                                     .current_position_um = 0x3456789a,
                                     .encoder_position_um = 0x05803931,
                                     .position_flags = 0x3,
                                     .ack_id = 1};
        auto arr = std::array<uint8_t, 16>{};
        auto body = std::span{arr};
        WHEN("serialized") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is written into the buffer correctly") {
                REQUIRE(body.data()[0] == 0xde);
                REQUIRE(body.data()[1] == 0xad);
                REQUIRE(body.data()[2] == 0xbe);
                REQUIRE(body.data()[3] == 0xef);
                REQUIRE(body.data()[4] == 1);
                REQUIRE(body.data()[5] == 2);
                REQUIRE(body.data()[6] == 0x34);
                REQUIRE(body.data()[7] == 0x56);
                REQUIRE(body.data()[8] == 0x78);
                REQUIRE(body.data()[9] == 0x9a);
                REQUIRE(body.data()[10] == 0x05);
                REQUIRE(body.data()[11] == 0x80);
                REQUIRE(body.data()[12] == 0x39);
                REQUIRE(body.data()[13] == 0x31);
                REQUIRE(body.data()[14] == 0x3);
                REQUIRE(body.data()[15] == 1);
            }
            THEN("size must be returned") { REQUIRE(size == 16); }
        }
    }

    GIVEN("a device info response message") {
        auto message =
            DeviceInfoResponse{.message_index = 0xdeadbeef,
                               .version = 0x00220033,
                               .flags = 0x11445566,
                               .shortsha{"abcdef0"},
                               .primary_revision = revision_get()->primary,
                               .secondary_revision = revision_get()->secondary,
                               .device_subidentifier = 0x9};
        message.tertiary_revision[0] = '.';
        message.tertiary_revision[1] = '2';
        auto arr = std::array<uint8_t, 30>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized into a buffer too small for its values") {
            auto size = message.serialize(arr.begin(), arr.begin() + 14);
            THEN("it is written into the buffer to its end") {
                REQUIRE(body.data()[0] == 0xde);
                REQUIRE(body.data()[1] == 0xad);
                REQUIRE(body.data()[2] == 0xbe);
                REQUIRE(body.data()[3] == 0xef);
                REQUIRE(body.data()[4] == 0x00);
                REQUIRE(body.data()[5] == 0x22);
                REQUIRE(body.data()[6] == 0x00);
                REQUIRE(body.data()[7] == 0x33);
                REQUIRE(body.data()[8] == 0x11);
                REQUIRE(body.data()[9] == 0x44);
                REQUIRE(body.data()[10] == 0x55);
                REQUIRE(body.data()[11] == 0x66);
                REQUIRE(body.data()[12] == 'a');
                REQUIRE(body.data()[13] == 'b');
            }
            THEN("it does not write past the end of the buffer") {
                for (uint32_t i = 14; i < arr.size(); i++) {
                    REQUIRE(body.data()[i] == 0);
                }
            }
            THEN("size must be returned") { REQUIRE(size == 14); }
        }
        WHEN("serialized into a buffer larger than needed") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is fully written into the buffer") {
                REQUIRE(body.data()[0] == 0xde);
                REQUIRE(body.data()[1] == 0xad);
                REQUIRE(body.data()[2] == 0xbe);
                REQUIRE(body.data()[3] == 0xef);
                REQUIRE(body.data()[4] == 0x00);
                REQUIRE(body.data()[5] == 0x22);
                REQUIRE(body.data()[6] == 0x00);
                REQUIRE(body.data()[7] == 0x33);
                REQUIRE(body.data()[8] == 0x11);
                REQUIRE(body.data()[9] == 0x44);
                REQUIRE(body.data()[10] == 0x55);
                REQUIRE(body.data()[11] == 0x66);
                REQUIRE(body.data()[12] == 'a');
                REQUIRE(body.data()[13] == 'b');

                REQUIRE(body.data()[14] == 'c');
                REQUIRE(body.data()[15] == 'd');
                REQUIRE(body.data()[16] == 'e');
                REQUIRE(body.data()[17] == 'f');
                REQUIRE(body.data()[18] == '0');
                REQUIRE(body.data()[19] == '\0');

                REQUIRE(body.data()[20] == revision_get()->primary);
                REQUIRE(body.data()[21] == revision_get()->secondary);
                REQUIRE(body.data()[22] == '.');
                REQUIRE(body.data()[23] == '2');

                REQUIRE(body.data()[24] == 0x9);
            }
            THEN("it does not write past the end of the buffer") {
                REQUIRE(body.data()[25] == 0);
            }
            THEN("size must be returned") { REQUIRE(size == 25); }
        }
    }

    GIVEN("an encoder position response") {
        auto message = MotorPositionResponse{.message_index = 0x1234,
                                             .current_position = 0xBA12CD,
                                             .encoder_position = 0xABEF,
                                             .position_flags = 0x2};
        auto arr =
            std::array<uint8_t, 14>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized into a buffer larger than needed") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is fully written into the buffer") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x00);
                REQUIRE(body.data()[2] == 0x12);
                REQUIRE(body.data()[3] == 0x34);
                REQUIRE(body.data()[4] == 0x00);
                REQUIRE(body.data()[5] == 0xBA);
                REQUIRE(body.data()[6] == 0x12);
                REQUIRE(body.data()[7] == 0xCD);
                REQUIRE(body.data()[8] == 0x00);
                REQUIRE(body.data()[9] == 0x00);
                REQUIRE(body.data()[10] == 0xAB);
                REQUIRE(body.data()[11] == 0xEF);
                REQUIRE(body.data()[12] == 0x02);
                REQUIRE(body.data()[13] == 0x00);
            }
            THEN("size must be returned") { REQUIRE(size == 13); }
        }
    }

    GIVEN("an update motor position response") {
        auto message =
            UpdateMotorPositionEstimationResponse{.message_index = 0x1234,
                                                  .current_position = 0xBA12CD,
                                                  .encoder_position = 0xABEF,
                                                  .position_flags = 0x2};
        auto arr =
            std::array<uint8_t, 14>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized into a buffer larger than needed") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is fully written into the buffer") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x00);
                REQUIRE(body.data()[2] == 0x12);
                REQUIRE(body.data()[3] == 0x34);
                REQUIRE(body.data()[4] == 0x00);
                REQUIRE(body.data()[5] == 0xBA);
                REQUIRE(body.data()[6] == 0x12);
                REQUIRE(body.data()[7] == 0xCD);
                REQUIRE(body.data()[8] == 0x00);
                REQUIRE(body.data()[9] == 0x00);
                REQUIRE(body.data()[10] == 0xAB);
                REQUIRE(body.data()[11] == 0xEF);
                REQUIRE(body.data()[12] == 0x02);
                REQUIRE(body.data()[13] == 0x00);
            }
            THEN("size must be returned") { REQUIRE(size == 13); }
        }
    }

    GIVEN("a read from eeprom response") {
        auto data = std::array<uint8_t, 5>{0, 1, 2, 3, 4};
        auto message = ReadFromEEPromResponse::create(
            0xdeadbeef, 13, data.cbegin(), data.cend());

        THEN("the length is correctly set.") {
            REQUIRE(message.data_length == data.size());
        }

        WHEN("serialized into too small a buffer") {
            auto arr = std::array<uint8_t, 10>{};
            auto size = message.serialize(arr.begin(), arr.end());
            auto iter = arr.begin();
            THEN("it is written into the buffer.") {
                uint32_t msg_ind;
                eeprom::types::address addr;
                eeprom::types::data_length len;
                // fetch message_index from serialize message
                iter = bit_utils::bytes_to_int(iter, iter + sizeof(uint32_t),
                                               msg_ind);
                // fetch address from serialized message
                iter = bit_utils::bytes_to_int(
                    iter, iter + sizeof(eeprom::types::address), addr);
                // fetch length from serialized message
                iter = bit_utils::bytes_to_int(
                    iter, iter + sizeof(eeprom::types::data_length), len);
                // Check message index was serialized correctly
                REQUIRE(msg_ind == 0xdeadbeef);
                // check address was serialized correctly
                REQUIRE(addr == 13);
                // check length was serialized correctly
                REQUIRE(len == sizeof(data));
                // check that the data has been serialized till the end of the
                // buffer
                for (size_t i = 0; i < data.size() && iter < arr.end(); i++) {
                    REQUIRE(*(iter++) == data[i]);
                }
            }
            THEN("size is correct") { REQUIRE(size == 10); }
        }

        WHEN("serialized into too large a buffer") {
            auto arr = std::array<uint8_t, ReadFromEEPromResponse::SIZE + 5>{};
            auto size = message.serialize(arr.begin(), arr.end());
            auto iter = arr.begin();
            THEN("it is written into the buffer.") {
                uint32_t msg_ind;
                eeprom::types::address addr;
                eeprom::types::data_length len;
                // fetch message_index from serialized message
                iter = bit_utils::bytes_to_int(iter, iter + sizeof(uint32_t),
                                               msg_ind);
                // fetch address from serialized message
                iter = bit_utils::bytes_to_int(
                    iter, iter + sizeof(eeprom::types::address), addr);
                // fetch length from serialized message
                iter = bit_utils::bytes_to_int(
                    iter, iter + sizeof(eeprom::types::data_length), len);
                // check message_index was serialized correctly
                REQUIRE(msg_ind == 0xdeadbeef);
                // check address was serialized correctly
                REQUIRE(addr == 13);
                // check length was serialized correctly
                REQUIRE(len == sizeof(data));
                // check that the data has been serialized
                for (size_t i = 0; i < data.size() && iter < arr.end(); i++) {
                    REQUIRE(*(iter++) == data[i]);
                }
                // check that the remainder of the buffer is 0x00
                for (size_t i = data.size();
                     i < eeprom::types::max_data_length && iter < arr.end();
                     i++) {
                    REQUIRE(*(iter++) == 0x00);
                }
            }
            THEN("size is correct") {
                REQUIRE(size == ReadFromEEPromResponse::SIZE);
            }
        }
    }

    GIVEN("a tip action response") {
        constexpr uint8_t MESSAGE_SIZE = 19;
        auto message =
            TipActionResponse{.message_index = 0x1234,
                              .group_id = 1,
                              .seq_id = 2,
                              .current_position_um = 0x3456789a,
                              .encoder_position_um = 0x05803931,
                              .position_flags = 0x0,
                              .ack_id = 0x1,
                              .action = can::ids::PipetteTipActionType::clamp,
                              .success = 0x1,
                              .gear_motor_id = can::ids::GearMotorId::left};
        auto arr = std::array<uint8_t, MESSAGE_SIZE + 5>{0, 0, 0, 0, 0, 0, 0, 0,
                                                         0, 0, 0, 0, 0, 0, 0, 0,
                                                         0, 0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized into a buffer exactly the size needed") {
            auto size = message.serialize(arr.begin(), arr.end());
            THEN("it is fully written into the buffer") {
                REQUIRE(body.data()[0] == 0x0);
                REQUIRE(body.data()[1] == 0x0);
                REQUIRE(body.data()[2] == 0x12);
                REQUIRE(body.data()[3] == 0x34);
                REQUIRE(body.data()[4] == 0x01);
                REQUIRE(body.data()[5] == 0x02);
                REQUIRE(body.data()[6] == 0x34);
                REQUIRE(body.data()[7] == 0x56);
                REQUIRE(body.data()[8] == 0x78);
                REQUIRE(body.data()[9] == 0x9a);
                REQUIRE(body.data()[10] == 0x05);
                REQUIRE(body.data()[11] == 0x80);
                REQUIRE(body.data()[12] == 0x39);
                REQUIRE(body.data()[13] == 0x31);
                REQUIRE(body.data()[14] == 0x0);
                REQUIRE(body.data()[15] == 0x1);
                REQUIRE(body.data()[16] == 0x0);
                REQUIRE(body.data()[17] == 0x1);
            }
            THEN("the total message size is the expected value") {
                REQUIRE(size == MESSAGE_SIZE);
            }
        }
    }
}
