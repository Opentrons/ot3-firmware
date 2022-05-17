#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"

using namespace lms;

TEST_CASE("Linear motion system using a belt") {
    GIVEN("OT2 X,Y gantry config") {
        struct LinearMotionSystemConfig<BeltConfig> linearConfig {
            .mech_config = BeltConfig{.pulley_diameter = 12.7},
            .steps_per_rev = 200, .microstep = 32, .encoder_ppr = 1000,
        };
        REQUIRE(linearConfig.get_steps_per_mm() == 160.40813f);
        REQUIRE(linearConfig.get_encoder_pulses_per_mm() ==
                100.25508f);
    }
}
