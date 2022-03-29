#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"

using namespace lms;

TEST_CASE("Linear motion system using a leadscrew") {
    GIVEN("OT2 GEN2 pipette config") {
        struct LinearMotionSystemConfig<LeadScrewConfig> linearConfig {
            .mech_config = LeadScrewConfig{.lead_screw_pitch = 2},
            .steps_per_rev = 200, .microstep = 32,
        };
        THEN("the steps/mm calculation should match the known value") {
            REQUIRE(linearConfig.get_steps_per_mm() == 3200);
        }
        THEN("the um/step calculation should match the known value") {
            REQUIRE(linearConfig.get_um_per_step() == 0.3125);
        }
    }
}
