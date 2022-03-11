#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"

using namespace lms;

TEST_CASE("Linear motion system using a belt") {
    GIVEN("OT2 X,Y gantry config") {
        struct LinearMotionSystemConfig<BeltConfig> linearConfig {
            .mech_config = BeltConfig{.pulley_diameter = 12.7},
            .steps_per_rev = 200, .microstep = 32,
        };
        REQUIRE(linearConfig.get_steps_per_mm() == 160.40813f);
    }
}
