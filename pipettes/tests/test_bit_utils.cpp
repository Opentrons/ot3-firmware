#include <array>
#include <span>

#include "bit_utils.hpp"
#include "catch2/catch.hpp"

SCENARIO("bytes_to_int works") {
    GIVEN("a 2 byte span") {
        auto arr = std::array{1, 2};
        std::span sp{arr};
        WHEN("called") {
            uint16_t val = 0;
            bit_utils::bytes_to_int<uint16_t , int>(sp, val);
            THEN("it is converted to a uint16_t") { REQUIRE(val == 0x0102); }
        }
    }
    GIVEN("a 4 byte span") {
        auto arr = std::array{0xFF, 0xef, 0x3, 0x1};
        std::span sp{arr};
        WHEN("called") {
            uint32_t val = 0;
            bit_utils::bytes_to_int<uint32_t , int>(sp, val);
            THEN("it is converted to a uint32_t") { REQUIRE(val == 0xFFEF0301); }
        }
    }
    GIVEN("a 1 byte span") {
        auto arr = std::array{0xdd};
        std::span sp{arr};
        WHEN("called") {
            uint8_t val = 0;
            bit_utils::bytes_to_int<uint8_t , int>(sp, val);
            THEN("it is converted to a uint8_t") { REQUIRE(val == 0xDD); }
        }
    }
}
