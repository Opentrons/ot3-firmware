#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "common/tests/mock_message_queue.hpp"
#include "motor-control/core/brushed_motor/brushed_motor_interrupt_handler.hpp"
#include "motor-control/core/brushed_motor/error_tolerance_config.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "motor-control/tests/mock_brushed_motor_components.hpp"
#include "motor-control/tests/mock_move_status_reporter_client.hpp"

using namespace brushed_motor_handler;

auto _gear_config = lms::LinearMotionSystemConfig<lms::GearBoxConfig>{
    .mech_config =
        lms::GearBoxConfig{.gear_diameter = 9, .gear_reduction_ratio = 84.29},
    .steps_per_rev = 0,
    .microstep = 0,
    .encoder_pulses_per_rev = 512};
auto _error_config =
    error_tolerance_config::BrushedMotorErrorTolerance{_gear_config, 32000};

SCENARIO("testing error tolerance handling") {
    GIVEN("a motion controller and motor interrupt handler") {
        test_mocks::MockBrushedMotionController controller(_error_config);
        auto pos_error_before = _error_config.acceptable_position_error;
        auto unwanted_movement_before =
            _error_config.unwanted_movement_threshold;
        WHEN(
            "a set gripper error tolerance request is handled by the motion "
            "controller") {
            auto msg = can::messages::SetGripperErrorToleranceRequest{
                .message_index = 0,
                .max_pos_error_mm = 98304,
                .max_unwanted_movement_mm = 98304,
            };
            controller.set_error_tolerance(msg);
            THEN("error config values should be updated") {
                CHECK(pos_error_before !=
                      _error_config.acceptable_position_error);
                CHECK(unwanted_movement_before !=
                      _error_config.unwanted_movement_threshold);
                CHECK(_error_config.acceptable_position_error ==
                      int(_gear_config.get_encoder_pulses_per_mm() * 1.5));
                CHECK(_error_config.unwanted_movement_threshold ==
                      int(_gear_config.get_encoder_pulses_per_mm() * 1.5));
            }
        }
    }
}

SCENARIO("testing brushed motor holdoff value") {
    GIVEN("a motion controller and motor interrupt handler") {
        test_mocks::MockBrushedMotionController controller(_error_config);
        CHECK(_error_config.get_holdoff_ms() == 1.0);
        WHEN("a set idle holdoff request is handled") {
            auto msg = can::messages::SetGripperJawHoldoffRequest{
                .message_index = 0,
                .holdoff_ms = 1310720,  // 20 ms
            };
            controller.set_idle_holdoff(msg);
            THEN("idle holdoff value should be updated in error config") {
                CHECK(_error_config.get_holdoff_ms() == 20.0);
                CHECK(_error_config.idle_holdoff_ticks == 640);

                THEN("controller should get the correct holdoff ms value") {
                    CHECK(controller.get_idle_holdoff_ms() == 1310720);
                }
            }
        }
    }
}