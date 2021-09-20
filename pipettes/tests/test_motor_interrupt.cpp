#include "catch2/catch.hpp"

#include "motor-control/core/motor_interrupt_handler.hpp"

SCENARIO("read and write pipette serial numbers") {
    GIVEN("write command") {
    const uint8_t DUMMY_VALUE = 0x2;

    TestI2C testI2C{};

    WHEN("i2c is initialized") {
        THEN("the current value stored should be zero") {
            const uint8_t ZERO_VALUE = 0x0;
            REQUIRE(ZERO_VALUE == testI2C.stored);
        }
    }

    WHEN("the command is received") {
        THEN("the value should be stored") {
            write(testI2C, DUMMY_VALUE);
            REQUIRE(DUMMY_VALUE == testI2C.stored);
        }
    }

    WHEN("the value is queried") {
        THEN("the stored value should be returned") {
            const uint8_t received_buff = read(testI2C);
            REQUIRE(DUMMY_VALUE == received_buff);
            }
        }
    }

    GIVEN("A new device") {

    }
}