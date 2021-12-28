#include "catch2/catch.hpp"
#include "common/simulation/eeprom.hpp"
#include "pipettes/core/eeprom.hpp"

using namespace eeprom;

SCENARIO("read and write pipette serial numbers") {
    GIVEN("write command") {
        const uint8_t DUMMY_VALUE = 0x2;

        auto sim_i2c = sim_i2c::SimEEProm{};
        auto subject = eeprom::EEPromWriter{sim_i2c};

        WHEN("i2c is initialized") {
            THEN("the current value stored should be zero") {
                const uint8_t ZERO_VALUE = 0x0;
                REQUIRE(ZERO_VALUE == subject.read());
            }
        }

        WHEN("writing a new value") {
            subject.write(DUMMY_VALUE);
            THEN("the read value matches what eas written") {
                REQUIRE(DUMMY_VALUE == subject.read());
            }
        }
    }
}