#include "catch2/catch.hpp"
#include "motor-control/tests/mock_motor_hardware.hpp"

// This file only tests functions in the StepperMotorHardwareIface that are
// non-virtual, and therefore shared by all stepper hardware implementations.

TEST_CASE("position tracker functionality") {
    GIVEN("a motor hardware interface") {
        test_mocks::MockMotorHardware hardware;
        THEN("the position tracker is initialized to 0") {
            REQUIRE(hardware.get_step_tracker() == 0);
        }
        WHEN("changing the position tracker") {
            hardware.set_step_tracker(20);
            THEN("getting the position tracker returns the right value") {
                REQUIRE(hardware.get_step_tracker() == 20);
            }
            AND_THEN("resetting the position tracker") {
                hardware.reset_step_tracker();
                THEN("it is reset to 0") {
                    REQUIRE(hardware.get_step_tracker() == 0);
                }
            }
        }
    }
}