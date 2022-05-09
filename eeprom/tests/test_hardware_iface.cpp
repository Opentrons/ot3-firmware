#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/hardware_iface.hpp"

struct MockEepromHardwareIface : public eeprom::hardware_iface::EEPromHardwareIface {
    void set_write_protect(bool enabled) { set_calls.push_back(enabled); }
    std::vector<bool> set_calls{};
};

SCENARIO("WriteProtector class") {
    GIVEN("A write protector") {
        auto hw = MockEepromHardwareIface{};
        auto subject = eeprom::hardware_iface::WriteProtector(hw);
        WHEN("disable is called") {
            subject.disable();
            THEN("write protect is disabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{false});
            }
        }
        WHEN("disable then enable is called") {
            subject.disable();
            subject.enable();
            THEN("write protect is disabled then enabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("disable is called twice then enable is called once") {
            subject.disable();
            subject.disable();
            subject.enable();
            THEN("write protect is disabled and not reenabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{false});
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
                REQUIRE(hw.set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("enable is only  call") {
            subject.enable();
            THEN("write protect is enabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{true});
            }
        }

        WHEN("enable first call") {
            subject.enable();
            subject.disable();
            THEN("write protect is enabled then disabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{true, false});
            }
        }
    }
}
