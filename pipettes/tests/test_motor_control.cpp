#include "catch2/catch.hpp"

#include <array>
#include <span>

#include "catch2/catch.hpp"
#include "pipettes/tests/test_motor_control.hpp"


SCENARIO("test commands are formatted correctly") {
    GIVEN("the setup command") {
        auto mc = TestMotorControl();
        mc.setup();
        const uint8_t status_value = 0x0;
        const uint32_t data_value = 0x0;
        WHEN("Setup is run, data and status should mimic the value sent by the command") {
            THEN("data and status should be equal to x and y respectively") {
                REQUIRE(mc.data == data_value);
                REQUIRE(mc.status == status_value);
            }
        }
    }

    GIVEN("a write command") {
    }

}
