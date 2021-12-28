#include "catch2/catch.hpp"
#include "pipettes/core/eeprom.hpp"
#include "common/simulation/i2c.hpp"

using namespace eeprom;

SCENARIO("read and write pipette serial numbers") {
    GIVEN("write command") {
        const uint8_t DUMMY_VALUE = 0x2;

        auto sim_i2c = i2c::SimI2C{};
        auto testI2C = eeprom::EEPromWriter{sim_i2c};

        WHEN("i2c is initialized") {
            THEN("the current value stored should be zero") {
                const uint8_t ZERO_VALUE = 0x0;
                REQUIRE(ZERO_VALUE == read(testI2C));
            }
        }

        WHEN("writing a new value") {
            write(testI2C, DUMMY_VALUE);
            THEN("the read value matches what eas written") {
                REQUIRE(DUMMY_VALUE == read(testI2C));
            }
        }
    }
}