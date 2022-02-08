#include <array>
#include <cstdint>
#include <vector>

#include "bootloader/core/util.h"
#include "catch2/catch.hpp"

using Result = std::pair<uint32_t, uint64_t>;
static auto results = std::vector<Result>{};

auto dword_address_iter_cb(uint32_t address, uint64_t data) -> void {
    results.push_back(Result{address, data});
}

SCENARIO("dword_address_iter") {
    GIVEN("a data buffer on 64bit boundary") {
        auto arr = std::array<uint8_t, 16>{
            0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8,
            0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7
        };
        WHEN("called") {
            dword_address_iter(0x800000, arr.data(), arr.size(),
                               dword_address_iter_cb);

            THEN("it has one result") { REQUIRE(results.size() == 2); }
            THEN("the address and data are correct") {
                auto expected =
                    std::vector<Result>{
                        Result{0x800000, 0xFFFEFDFCFBFAF9F8},
                        Result{0x800008, 0xFEFDFCFBFAF9F8F7}
                    };
                REQUIRE(results == expected);
            }
        }
    }

    GIVEN("a data buffer shofter than 64bits boundary") {
        auto arr = std::array<uint8_t, 3>{
            0xFF, 0xFE, 0xFD
        };
        WHEN("called") {
            dword_address_iter(0x800000, arr.data(), arr.size(),
                               dword_address_iter_cb);

            THEN("it has one result") { REQUIRE(results.size() == 1); }
            THEN("the address and data are correct") {
                auto expected =
                    std::vector<Result>{Result{0x800000, 0xFFFEFD0000000000}};
                REQUIRE(results == expected);
            }
        }
    }

    GIVEN("a data buffer not on 64bit boundary") {
        auto arr = std::array<uint8_t, 18>{
            0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8,
            0xFE, 0xFD, 0xFC, 0xFB, 0xFA, 0xF9, 0xF8, 0xF7,
            0xFD, 0xFC
        };
        WHEN("called") {
            dword_address_iter(0x800000, arr.data(), arr.size(),
                               dword_address_iter_cb);

            THEN("it has one result") { REQUIRE(results.size() == 3); }
            THEN("the address and data are correct") {
                auto expected =
                    std::vector<Result>{
                        Result{0x800000, 0xFFFEFDFCFBFAF9F8},
                        Result{0x800008, 0xFEFDFCFBFAF9F8F7},
                        Result{0x800008, 0xFDFC000000000000}
                    };
                REQUIRE(results == expected);
            }
        }
    }

}