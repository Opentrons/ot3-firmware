#include <cstring>

#include "catch2/catch.hpp"
#include "common/core/bit_utils.hpp"
#include "common/core/version.h"
#include "rear-panel/core/bin_msg_ids.hpp"
#include "rear-panel/core/messages.hpp"

using namespace rearpanel::messages;

SCENARIO("message deserializing works") {
    GIVEN("a get device info request body") {
        auto arr = std::array<uint8_t, 4>{0x00, 0x03, 0x00, 0x00};
        WHEN("constructed") {
            auto r = DeviceInfoRequest::parse(arr.begin(), arr.end());
            THEN("it is converted to a the correct structure") {
                REQUIRE(r.length == 0x0000);
            }
            THEN("it can be compared for equality") {
                auto other = r;
                REQUIRE(other == r);
                other.length = 10;
                REQUIRE(other != r);
            }
        }
    }
}

SCENARIO("message serializing works") {
    GIVEN("a device info response message") {
        auto message =
            DeviceInfoResponse{.length = DeviceInfoResponse::get_length(),
                               .version = 0x00220033,
                               .flags = 0x11445566,
                               .shortsha{"abcdef0"},
                               .primary_revision = 'A',
                               .secondary_revision = 'B',
                               .tertiary_revision{"C"}};
        auto arr = std::array<uint8_t, 30>{0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                           0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        auto body = std::span{arr};
        WHEN("serialized into a buffer too small for its values") {
            auto next_free = message.serialize(arr.begin(), arr.begin() + 14);
            THEN("it is written into the buffer to its end") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x04);
                REQUIRE(body.data()[2] == 0x00);
                REQUIRE(body.data()[3] == 0x10);
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
            THEN("size must be returned") {
                REQUIRE(next_free == arr.begin() + 14);
            }
        }
        WHEN("serialized into a buffer larger than needed") {
            auto next_free = message.serialize(arr.begin(), arr.end());
            THEN("it is fully written into the buffer") {
                REQUIRE(body.data()[0] == 0x00);
                REQUIRE(body.data()[1] == 0x04);
                REQUIRE(body.data()[2] == 0x00);
                REQUIRE(body.data()[3] == 0x10);
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
                REQUIRE(body.data()[19] == 0);
                REQUIRE(body.data()[20] == 'A');
                REQUIRE(body.data()[21] == 'B');
                REQUIRE(body.data()[22] == 'C');
            }
            THEN("it does not write past the end of the buffer") {
                REQUIRE(body.data()[23] == 0);
            }
            THEN("size must be returned") {
                REQUIRE(next_free == arr.begin() + 24);
            }
        }
    }
}
