#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"

using namespace lms;

TEST_CASE("Linear motion system using a belt") {
    GIVEN("OT2 X,Y head config") {
        struct LinearMotionSystemConfig<BeltConfig> linearConfig {
            .mech_config =
                BeltConfig{.belt_pitch = 2, .pulley_tooth_count = 20},
            .steps_per_rev = 200, .microstep = 16,
        };
        REQUIRE(linearConfig.get_steps_per_mm() == 80);
    }
}
