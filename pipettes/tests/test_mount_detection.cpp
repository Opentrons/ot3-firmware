#include "can/core/ids.hpp"
#include "catch2/catch.hpp"
#include "pipettes/core/mount_sense.hpp"

SCENARIO("Initial mount detection") {
    GIVEN("A mount detector") {
        WHEN("Checking left-side voltages") {
            // Use a list of voltages that are rough midpoints of Z mount bounds
            auto voltage = GENERATE(1490, 1430, 450, 920);
            THEN("the mount should always be left") {
                INFO("Checking voltage " << voltage);
                REQUIRE(pipette_mounts::decide_id(voltage) ==
                        can::ids::NodeId::pipette_left);
            }
        }
        WHEN("Checking right-side voltages") {
            // Roughly the midpoints of A mount bounds
            auto voltage = GENERATE(2375, 2852);
            THEN("the mount should always be right") {
                INFO("Checking voltage " << voltage);
                REQUIRE(pipette_mounts::decide_id(voltage) ==
                        can::ids::NodeId::pipette_right);
            }
        }
        WHEN("Checking out of bounds voltages") {
            auto voltage = GENERATE(3001, 4095);
            THEN("the mount should always be left") {
                INFO("Checking voltage " << voltage);
                REQUIRE(pipette_mounts::decide_id(voltage) ==
                        can::ids::NodeId::pipette_left);
            }
        }
    }
}
