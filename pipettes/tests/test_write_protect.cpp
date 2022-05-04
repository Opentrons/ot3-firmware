#include "catch2/catch.hpp"
#include "eeprom/core/write_protect.hpp"
#include <vector>

static auto set_calls = std::vector<bool>{};

void eeprom::write_protect::set(bool enabled) {
    set_calls.push_back(enabled);
}


SCENARIO("WriteProtector class") {
    GIVEN("A write protector") {
        auto subject = eeprom::write_protect::WriteProtector();
        WHEN("disable is called") {
            set_calls.clear();

            subject.disable();
            THEN("write protect is disabled") {
                REQUIRE(set_calls == std::vector<bool>{false});
            }
        }
        WHEN("disable then enable is called") {
            set_calls.clear();

            subject.disable();
            subject.enable();
            THEN("write protect is disabled then enabled") {
                REQUIRE(set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("disable is called twice then enable is called once") {
            set_calls.clear();

            subject.disable();
            subject.disable();
            subject.enable();
            THEN("write protect is disabled and not reenabled") {
                REQUIRE(set_calls == std::vector<bool>{false});
            }
        }

        WHEN("enable is called after a series of disable and enable calls") {
            set_calls.clear();

            subject.disable();
            subject.disable();
            subject.enable();
            subject.disable();
            subject.enable();
            subject.enable();
            THEN("write protect is disabled and re enabled") {
                REQUIRE(set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("enable is only  call") {
            set_calls.clear();

            subject.enable();
            THEN("write protect is enabled") {
                REQUIRE(set_calls == std::vector<bool>{true});
            }
        }

        WHEN("enable first call") {
            set_calls.clear();

            subject.enable();
            subject.disable();
            THEN("write protect is enabled then disabled") {
                REQUIRE(set_calls == std::vector<bool>{true, false});
            }
        }
    }
}
