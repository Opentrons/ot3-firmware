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
#include "motor-control/core/tasks/move_status_reporter_task.hpp"

using namespace motor_messages;
using namespace move_status_reporter_task;
using namespace lms;

struct MockCanClient {
    std::deque<std::pair<can::ids::NodeId, can::messages::ResponseMessageType>>
        queue{};
    auto send_can_message(can::ids::NodeId node_id,
                          const can::messages::ResponseMessageType& m) -> void {
        queue.push_back(std::make_pair(node_id, m));
    }
};

SCENARIO("testing move status position translation") {
    GIVEN("a status handler with known LMS config") {
        struct LinearMotionSystemConfig<LeadScrewConfig> linearConfig {
            .mech_config = LeadScrewConfig{.lead_screw_pitch = 2},
            .steps_per_rev = 200, .microstep = 32, .encoder_ppr = 1000,
        };
        auto mcc = MockCanClient();
        auto handler = MoveStatusMessageHandler(mcc, linearConfig);
        WHEN("passing a position of 0 steps with 0 encoder pulses") {
            handler.handle_message(Ack{.group_id = 11,
                                       .seq_id = 8,
                                       .current_position_steps = 0,
                                       .encoder_position = 0,
                                       .ack_id = AckMessageId::timeout});
            CHECK(mcc.queue.size() == 1);

            auto resp = mcc.queue.front();
            auto resp_msg = std::get<can::messages::MoveCompleted>(resp.second);
            THEN("the non-position values should be passed through") {
                REQUIRE(resp_msg.group_id == 11);
                REQUIRE(resp_msg.seq_id == 8);
                REQUIRE(resp_msg.ack_id == 3);
            }
            THEN("the position value should still be 0") {
                REQUIRE(resp_msg.current_position_um == 0);
            }
            THEN("the encoder value should still be 0") {
                REQUIRE(resp_msg.encoder_position == 0);
            }
        }
        WHEN("passing a position of 0 steps with -100 encoder pulses") {
            handler.handle_message(Ack{.group_id = 11,
                                       .seq_id = 8,
                                       .current_position_steps = 0,
                                       .encoder_position = -1000,
                                       .ack_id = AckMessageId::timeout});
            CHECK(mcc.queue.size() == 1);

            auto resp = mcc.queue.front();
            auto resp_msg = std::get<can::messages::MoveCompleted>(resp.second);
            THEN("the non-position values should be passed through") {
                REQUIRE(resp_msg.group_id == 11);
                REQUIRE(resp_msg.seq_id == 8);
                REQUIRE(resp_msg.ack_id == 3);
            }
            THEN("the position value should still be 0") {
                REQUIRE(resp_msg.current_position_um == 0);
            }
            THEN("the encoder value should still be 0") {
                REQUIRE(resp_msg.encoder_position == -500);
            }
        }
        WHEN("passing a position of fullscale steps") {
            handler.handle_message(
                Ack{.group_id = 0,
                    .seq_id = 0,
                    .current_position_steps = UINT_MAX,
                    .ack_id = AckMessageId::complete_without_condition});
            CHECK(mcc.queue.size() == 1);
            auto resp = mcc.queue.front();
            auto resp_msg = std::get<can::messages::MoveCompleted>(resp.second);
            THEN("the position value should not be clipped") {
                REQUIRE(
                    resp_msg.current_position_um ==
                    static_cast<uint32_t>(
                        static_cast<double>(linearConfig.get_um_per_step()) *
                        static_cast<double>(UINT_MAX)));
            }
        }
        WHEN("passing a known input position in steps") {
            handler.handle_message(Ack{.group_id = 0,
                                       .seq_id = 0,
                                       .current_position_steps = 10000,
                                       .ack_id = AckMessageId::position_error});
            CHECK(mcc.queue.size() == 1);
            auto resp = mcc.queue.front();
            auto resp_msg = std::get<can::messages::MoveCompleted>(resp.second);
            THEN("the position in micrometers should be accurate") {
                REQUIRE(resp_msg.current_position_um == 3125);
            }
        }
    }
}
