#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"

using namespace lms;

TEST_CASE("Linear motion system using a leadscrew and gears") {
    GIVEN("Gripper Z config") {
        struct LinearMotionSystemConfig<LeadScrewConfig> linearConfig {
            .mech_config = LeadScrewConfig{.lead_screw_pitch = 12,
                                           .gear_reduction_ratio = 1.8},
            .steps_per_rev = 200, .microstep = 16,
            .encoder_pulses_per_rev = 1024.0,
        };
        REQUIRE(linearConfig.get_usteps_per_mm() == 479.99997f);
        REQUIRE(linearConfig.get_encoder_pulses_per_mm() == 614.39996f);
    }
}

TEST_CASE("Linear motion system using a brushed motor and gears") {
    GIVEN("Gripper G config") {
        struct LinearMotionSystemConfig<GearBoxConfig> linearConfig {
            .mech_config = GearBoxConfig{.gear_diameter = 9,
                                         .gear_reduction_ratio = 84.29},
            .steps_per_rev = 0, .microstep = 0, .encoder_pulses_per_rev = 512,
        };
        REQUIRE(linearConfig.get_usteps_per_mm() == 0);
        REQUIRE(linearConfig.get_encoder_pulses_per_mm() == 6105.39307f);
    }
}
