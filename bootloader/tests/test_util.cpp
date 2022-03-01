#include <array>
#include <cstdint>
#include <vector>

#include "bootloader/core/util.h"
#include "catch2/catch.hpp"

using Result = std::pair<uint32_t, uint64_t>;
static auto results = std::vector<Result>{};

auto dword_address_iter_cb(uint32_t address, uint64_t data) -> bool {
    results.push_back(Result{address, data});
    return true;
}

SCENARIO("dword_address_iter") {
    GIVEN("a data buffer on 64bit boundary") {
        auto arr = std::array<uint8_t, 16>{0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA,
                                           0xF9, 0xF8, 0xFE, 0xFD, 0xFC, 0xFB,
                                           0xFA, 0xF9, 0xF8, 0xF7};
        WHEN("called") {
            results.clear();
            auto ret = dword_address_iter(0x800000, arr.data(), arr.size(),
                                          dword_address_iter_cb);

            auto expected =
                std::vector<Result>{Result{0x800000, 0xF8F9FAFBFCFDFEFF},
                                    Result{0x800008, 0xF7F8F9FAFBFCFDFE}};

            THEN("returns true") { REQUIRE(ret); }
            THEN("the address and data are correct") {
                REQUIRE(results == expected);
            }
        }
    }

    GIVEN("a data buffer shorter than 64bit boundary") {
        auto arr = std::array<uint8_t, 3>{0xFF, 0xFE, 0xFD};
        WHEN("called") {
            results.clear();
            auto ret = dword_address_iter(0x800000, arr.data(), arr.size(),
                                          dword_address_iter_cb);

            THEN("returns true") { REQUIRE(ret); }
            THEN("the address and data are correct") {
                auto expected =
                    std::vector<Result>{Result{0x800000, 0x0000000000FDFEFF}};
                REQUIRE(results == expected);
            }
        }
    }

    GIVEN("a data buffer not on 64bit boundary") {
        auto arr = std::array<uint8_t, 18>{0xFF, 0xFE, 0xFD, 0xFC, 0xFB, 0xFA,
                                           0xF9, 0xF8, 0xFE, 0xFD, 0xFC, 0xFB,
                                           0xFA, 0xF9, 0xF8, 0xF7, 0xFD, 0xFC};
        WHEN("called") {
            results.clear();
            auto ret = dword_address_iter(0x800000, arr.data(), arr.size(),
                                          dword_address_iter_cb);
            THEN("returns true") { REQUIRE(ret); }
            THEN("the address and data are correct") {
                auto expected =
                    std::vector<Result>{Result{0x800000, 0xF8F9FAFBFCFDFEFF},
                                        Result{0x800008, 0xF7F8F9FAFBFCFDFE},
                                        Result{0x800010, 0x000000000000FCFD}};
                REQUIRE(results == expected);
            }
        }
    }

    GIVEN("an empty data buffer") {
        WHEN("called") {
            results.clear();
            auto ret =
                dword_address_iter(0x800000, nullptr, 0, dword_address_iter_cb);

            THEN("returns true") { REQUIRE(ret); }
            THEN("the address and data are correct") {
                auto expected = std::vector<Result>{};
                REQUIRE(results == expected);
            }
        }
    }
}