#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/hardware_iface.hpp"

class MockEepromHardwareIface
    : public eeprom::hardware_iface::EEPromHardwareIface {
    using eeprom::hardware_iface::EEPromHardwareIface::EEPromHardwareIface;

  public:
    void set_write_protect(bool enabled) { set_calls.push_back(enabled); }
    std::vector<bool> set_calls{};
};

SCENARIO("Configuring EEProm Address Length") {
    GIVEN("EEProm hw interface constructor") {
        WHEN("Default constructor used") {
            auto hw_default = MockEepromHardwareIface{};
            THEN("address setting is 8 bit") {
                REQUIRE(hw_default.get_eeprom_addr_bytes() ==
                        static_cast<size_t>(
                            eeprom::hardware_iface::EEPromAddressType::
                                EEPROM_ADDR_8_BIT));
            }
        }
        WHEN("Explicit 8 bit contructor used") {
            auto hw_8_bit_explicit = MockEepromHardwareIface(
                eeprom::hardware_iface::EEPromChipType::MICROCHIP_24AA02T);
            THEN("address setting is 8 bit") {
                REQUIRE(hw_8_bit_explicit.get_eeprom_addr_bytes() ==
                        static_cast<size_t>(
                            eeprom::hardware_iface::EEPromAddressType::
                                EEPROM_ADDR_8_BIT));
            }
        }
        WHEN("Explicit 16 bit contructor used") {
            auto hw_16_bit_explicit = MockEepromHardwareIface(
                eeprom::hardware_iface::EEPromChipType::ST_M24128);
            THEN("address setting is 16 bit") {
                REQUIRE(hw_16_bit_explicit.get_eeprom_addr_bytes() ==
                        static_cast<size_t>(
                            eeprom::hardware_iface::EEPromAddressType::
                                EEPROM_ADDR_16_BIT));
            }
        }
    }
}

SCENARIO("WriteProtector class") {
    GIVEN("A write protector") {
        auto hw = MockEepromHardwareIface{};
        WHEN("disable is called") {
            hw.disable();
            THEN("write protect is disabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{false});
            }
        }
        WHEN("disable then enable is called") {
            hw.disable();
            hw.enable();
            THEN("write protect is disabled then enabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("disable is called twice then enable is called once") {
            hw.disable();
            hw.disable();
            hw.enable();
            THEN("write protect is disabled and not reenabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{false});
            }
        }

        WHEN("enable is called after a series of disable and enable calls") {
            hw.disable();
            hw.disable();
            hw.enable();
            hw.disable();
            hw.enable();
            hw.enable();
            THEN("write protect is disabled and re enabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{false, true});
            }
        }

        WHEN("enable is only  call") {
            hw.enable();
            THEN("write protect is enabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{true});
            }
        }

        WHEN("enable first call") {
            hw.enable();
            hw.disable();
            THEN("write protect is enabled then disabled") {
                REQUIRE(hw.set_calls == std::vector<bool>{true, false});
            }
        }
    }
}
