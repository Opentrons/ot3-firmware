#include <array>

#include "can/core/ids.hpp"
#include "catch2/catch.hpp"
#include "motor-control/core/types.hpp"

using Flags = MotorPositionStatus::Flags;

TEST_CASE("motor status class functionality") {
    MotorPositionStatus status;
    GIVEN("motor flag object and a single flag") {
        auto flag =
            GENERATE(Flags::encoder_position_ok, Flags::stepper_position_ok);
        THEN("the value is 0x00") { REQUIRE(status.get_flags() == 0x00); }
        THEN("no flags are set") { REQUIRE(!status.check_flag(flag)); }
        WHEN("a flag is set") {
            status.set_flag(flag);
            THEN("the flag can be checked") {
                REQUIRE(status.check_flag(flag));
            }
            AND_THEN("the flag is cleared") {
                status.clear_flag(flag);
                THEN("the flag is no longer set") {
                    REQUIRE(!status.check_flag(flag));
                }
            }
        }
    }

    GIVEN("motor status object with multiple flags set") {
        status.set_flag(Flags::encoder_position_ok);
        status.set_flag(Flags::stepper_position_ok);
        THEN("all of the flags are set") {
            REQUIRE(status.get_flags() == 0x3);
            REQUIRE(status.check_flag(Flags::encoder_position_ok));
            REQUIRE(status.check_flag(Flags::stepper_position_ok));
        }
    }
}