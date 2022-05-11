#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"

using namespace lms;

TEST_CASE("Linear motion system using a leadscrew and gears") {
    GIVEN("Gripper Z config") {
        struct LinearMotionSystemConfig<LeadScrewConfig> linearConfig {
            .mech_config = LeadScrewConfig{.lead_screw_pitch = 4},
            .steps_per_rev = 200, .microstep = 16, .encoder_ppr = 0,
            .gear_ratio = 1.8,
        };
        REQUIRE(linearConfig.get_steps_per_mm() == 1440);
        REQUIRE(linearConfig.get_encoder_pulses_per_mm() == 0.0);
    }
}
