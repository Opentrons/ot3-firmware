#include "catch2/catch.hpp"
#include "eeprom/core/write_protect.hpp"
#include <vector>


struct MockWriteProtectPin : public eeprom::write_protect::WriteProtectPin {
    void set(bool enabled) {
        set_calls.push_back(enabled);
    }
    std::vector<bool> set_calls{};
};


SCENARIO("WriteProtector class") {
    GIVEN("A write protector") {
        auto pin = MockWriteProtectPin{};
        auto subject = eeprom::write_protect::WriteProtector(pin);
        WHEN("disable is called") {
            subject.disable();
            THEN("write protect is disabled") {
                REQUIRE(pin.set_calls == std::vector<bool>{false});
            }
        }
        WHEN("disable then enable is called") {
            subject.disable();
            subject.enable();
            THEN("write protect is disabled then enabled") {
                REQUIRE(pin.set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("disable is called twice then enable is called once") {
            subject.disable();
            subject.disable();
            subject.enable();
            THEN("write protect is disabled and not reenabled") {
                REQUIRE(pin.set_calls == std::vector<bool>{false});
            }
        }

        WHEN("enable is called after a series of disable and enable calls") {
            subject.disable();
            subject.disable();
            subject.enable();
            subject.disable();
            subject.enable();
            subject.enable();
            THEN("write protect is disabled and re enabled") {
                REQUIRE(pin.set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("enable is only  call") {
            subject.enable();
            THEN("write protect is enabled") {
                REQUIRE(pin.set_calls == std::vector<bool>{true});
            }
        }

        WHEN("enable first call") {
            subject.enable();
            subject.disable();
            THEN("write protect is enabled then disabled") {
                REQUIRE(pin.set_calls == std::vector<bool>{true, false});
            }
        }
    }
}
