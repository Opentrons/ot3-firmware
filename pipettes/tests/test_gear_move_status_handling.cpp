#include <climits>
#include <cstdint>
#include <deque>
#include <utility>
#include <variant>

#include "can/core/ids.hpp"
#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_messages.hpp"
#include "pipettes/core/tasks/gear_move_status_reporter_task.hpp"

using namespace motor_messages;
using namespace pipettes::tasks::gear_move_status;
using namespace lms;

struct MockCanClient {
    std::deque<std::pair<can::ids::NodeId, can::messages::ResponseMessageType>>
        queue{};
    auto send_can_message(can::ids::NodeId node_id,
                          const can::messages::ResponseMessageType& m) -> void {
        queue.push_back(std::make_pair(node_id, m));
    }
};

SCENARIO("testing gear move status response handling") {
    GIVEN("a left and right status handler with known LMS config") {
        struct LinearMotionSystemConfig<LeadScrewConfig> linearConfig {
            .mech_config = LeadScrewConfig{.lead_screw_pitch = 2},
            .steps_per_rev = 200, .microstep = 32,
            .encoder_pulses_per_rev = 1000,
        };
        auto left_ack = GearMotorAck{1,
                                     11,
                                     8,
                                     100,
                                     100,
                                     0x1,
                                     AckMessageId::complete_without_condition,
                                     can::ids::PipetteTipActionType::pick_up,
                                     can::ids::GearMotorId::left};

        auto mcc = MockCanClient();
        auto handler = MoveStatusMessageHandler(mcc, linearConfig);
        WHEN("Each handler is given the correct gear id") {
            handler.handle_message(left_ack);
            CHECK(mcc.queue.size() == 1);

            auto resp = mcc.queue.front();
            auto resp_msg =
                std::get<can::messages::TipActionResponse>(resp.second);
            THEN(
                "there should be a TipActionResponse and the id should match "
                "the handlers id") {
                REQUIRE(resp_msg.group_id == 11);
                REQUIRE(resp_msg.seq_id == 8);
                REQUIRE(resp_msg.gear_motor_id == can::ids::GearMotorId::left);
            }
        }
    }
}
