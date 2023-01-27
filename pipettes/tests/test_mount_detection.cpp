#include "can/core/ids.hpp"
#include "catch2/catch.hpp"
#include "pipettes/core/mount_sense.hpp"

SCENARIO("Initial mount detection") {
    GIVEN("A mount detector") {
        WHEN("Handling a high mount-id detect pin") {
            THEN("the mount should be left") {
                REQUIRE(pipette_mounts::decide_id(true) ==
                        can::ids::NodeId::pipette_left);
            }
        }
        WHEN("Checking a low mount-id detect pin") {
            THEN("the mount should always be right") {
                REQUIRE(pipette_mounts::decide_id(false) ==
                        can::ids::NodeId::pipette_right);
            }
        }
    }
}
