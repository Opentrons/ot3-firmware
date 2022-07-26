#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"

using namespace lms;

TEST_CASE("Linear motion system using a leadscrew and gears") {
    GIVEN("Gripper Z config") {
        struct LinearMotionSystemConfig<LeadScrewConfig> linearConfig {
            .mech_config = LeadScrewConfig{.lead_screw_pitch = 4},
            .steps_per_rev = 200, .microstep = 16, .encoder_pulses_per_rev = 0,
            .gear_ratio = 1.8,
        };
        REQUIRE(linearConfig.get_steps_per_mm() == 1440);
        REQUIRE(linearConfig.get_encoder_pulses_per_mm() == 0.0);
    }
}

TEST_CASE("Linear motion system using a brushed motor and gears") {
    GIVEN("Gripper G config") {
        struct LinearMotionSystemConfig<GearBoxConfig> linearConfig {
            .mech_config = GearBoxConfig{.gear_diameter = 9},
            .steps_per_rev = 0, .microstep = 0, .encoder_pulses_per_rev = 512,
            .gear_ratio = 84.29,
        };
        REQUIRE(linearConfig.get_steps_per_mm() == 0);
        REQUIRE(linearConfig.get_encoder_pulses_per_mm() == 6105.39307f);
    }
}
