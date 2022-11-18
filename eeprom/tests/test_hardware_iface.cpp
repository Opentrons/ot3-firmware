#include <vector>

#include "catch2/catch.hpp"
#include "eeprom/core/hardware_iface.hpp"

namespace eeprom {
namespace hardware_iface {

class MockEepromHardwareIface : public EEPromHardwareIface {
    using EEPromHardwareIface::EEPromHardwareIface;

  public:
    void set_write_protect(bool enabled) { set_calls.push_back(enabled); }
    std::vector<bool> set_calls{};
};

SCENARIO("Configuring EEProm Address Length") {
    GIVEN("EEProm hw interface constructor") {
        WHEN("Default constructor used") {
            auto hw_default = MockEepromHardwareIface{};
            THEN("address setting is 8 bit") {
                REQUIRE(
                    hw_default.get_eeprom_addr_bytes() ==
                    static_cast<size_t>(EEPromAddressType::EEPROM_ADDR_8_BIT));
            }
        }
        WHEN("Explicit 8 bit contructor used") {
            auto hw_8_bit_explicit =
                MockEepromHardwareIface(EEPromChipType::MICROCHIP_24AA02T);
            THEN("address setting is 8 bit") {
                REQUIRE(
                    hw_8_bit_explicit.get_eeprom_addr_bytes() ==
                    static_cast<size_t>(EEPromAddressType::EEPROM_ADDR_8_BIT));
            }
        }
        WHEN("Explicit 16 bit contructor used") {
            auto hw_16_bit_explicit =
                MockEepromHardwareIface(EEPromChipType::ST_M24128_BF);
            THEN("address setting is 16 bit") {
                REQUIRE(
                    hw_16_bit_explicit.get_eeprom_addr_bytes() ==
                    static_cast<size_t>(EEPromAddressType::EEPROM_ADDR_16_BIT));
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
SCENARIO("looking up i2c address by chip type") {
    WHEN("we have a MICROCHIP_24AA02T") {
        THEN("the address should be 0x50 shifted left (0xA0)") {
            REQUIRE(hardware_iface::get_i2c_device_address(
                        hardware_iface::EEPromChipType::MICROCHIP_24AA02T) ==
                    0xA0);
        }
    }
    WHEN("we have a ST_M24128_BF") {
        THEN("the address should be 0x50 shifted left (0xA0)") {
            REQUIRE(hardware_iface::get_i2c_device_address(
                        hardware_iface::EEPromChipType::ST_M24128_BF) == 0xA0);
        }
    }
    WHEN("we have a ST_M24128_DF") {
        THEN("the address should be 0x51 shifted left (0xA2)") {
            REQUIRE(hardware_iface::get_i2c_device_address(
                        hardware_iface::EEPromChipType::ST_M24128_DF) == 0xA2);
        }
    }
}

}  // namespace hardware_iface
}  // namespace eeprom
