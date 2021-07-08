#include <array>
#include <span>

#include "pipettes/core/bit_utils.hpp"
#include "catch2/catch.hpp"

SCENARIO("bytes_to_int works") {
    GIVEN("a 2 byte span") {
        auto arr = std::array{1, 2};
        std::span sp{arr};
        WHEN("called") {
            uint16_t val = 0;
            bit_utils::bytes_to_int<uint16_t, int>(sp, val);
            THEN("it is converted to a uint16_t") { REQUIRE(val == 0x0102); }
        }
    }
    GIVEN("a 4 byte span") {
        auto arr = std::array{0xFF, 0xef, 0x3, 0x1};
        std::span sp{arr};
        WHEN("called") {
            uint32_t val = 0;
            bit_utils::bytes_to_int<uint32_t, int>(sp, val);
            THEN("it is converted to a uint32_t") {
                REQUIRE(val == 0xFFEF0301);
            }
        }
    }
    GIVEN("a 1 byte span") {
        auto arr = std::array{0xdd};
        std::span sp{arr};
        WHEN("called") {
            uint8_t val = 0;
            bit_utils::bytes_to_int<uint8_t, int>(sp, val);
            THEN("it is converted to a uint8_t") { REQUIRE(val == 0xDD); }
        }
    }
}

SCENARIO("int_to_bytes works") {
    GIVEN("some integers") {
        auto arr = std::array<int, 7>{};
        uint32_t i32 = 0x01234567;
        uint16_t i16 = 0x89ab;
        uint8_t i8 = 0xcd;

        WHEN("called") {
            auto iter = arr.begin();
            iter = bit_utils::int_to_bytes(i32, iter);
            iter = bit_utils::int_to_bytes(i16, iter);
            iter = bit_utils::int_to_bytes(i8, iter);
            THEN("The values are written into the array") {
                REQUIRE(arr[0] == 0x01);
                REQUIRE(arr[1] == 0x23);
                REQUIRE(arr[2] == 0x45);
                REQUIRE(arr[3] == 0x67);
                REQUIRE(arr[4] == 0x89);
                REQUIRE(arr[5] == 0xab);
                REQUIRE(arr[6] == 0xcd);
            }
        }
    }
}
