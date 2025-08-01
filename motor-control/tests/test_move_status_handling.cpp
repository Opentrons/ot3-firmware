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
                          const can::messages::ResponseMessageType& m) -> bool {
        queue.push_back(std::make_pair(node_id, m));
        return true;
    }
};
struct MockUsageClient {
    std::deque<usage_storage_task::TaskMessage> queue{};
    auto send_usage_storage_queue(const usage_storage_task::TaskMessage& m)
        -> void {
        queue.push_back(m);
    }
};

SCENARIO("testing move status position translation") {
    GIVEN("a status handler with known LMS config") {
        struct LinearMotionSystemConfig<LeadScrewConfig> linearConfig {
            .mech_config = LeadScrewConfig{.lead_screw_pitch = 2,
                                           .gear_reduction_ratio = 1.0},
            .steps_per_rev = 200, .microstep = 32,
            .encoder_pulses_per_rev = 1000,
        };
        auto mcc = MockCanClient();
        auto muc = MockUsageClient();
        auto handler = MoveStatusMessageHandler(mcc, linearConfig, muc);
        WHEN("passing a position of 0 steps with 0 encoder pulses") {
            handler.handle_message(Ack{.group_id = 11,
                                       .seq_id = 8,
                                       .current_position_steps = 0,
                                       .encoder_position = 0,
                                       .position_flags = 0x3,
                                       .ack_id = AckMessageId::timeout,
                                       .start_encoder_position = 0,
                                       .usage_key = 1});
            CHECK(mcc.queue.size() == 1);
            CHECK(muc.queue.size() == 1);

            auto resp = mcc.queue.front();
            auto resp_msg = std::get<can::messages::MoveCompleted>(resp.second);
            THEN("the non-position values should be passed through") {
                REQUIRE(resp_msg.group_id == 11);
                REQUIRE(resp_msg.seq_id == 8);
                REQUIRE(resp_msg.ack_id == 3);
                REQUIRE(resp_msg.position_flags == 0x3);
            }
            THEN("the position value should still be 0") {
                REQUIRE(resp_msg.current_position_um == 0);
            }
            THEN("the encoder value should still be 0") {
                REQUIRE(resp_msg.encoder_position_um == 0);
            }

            auto usage_resp = muc.queue.front();
            auto usage_resp_msg =
                std::get<usage_messages::IncreaseDistanceUsage>(usage_resp);
            THEN("The distance traveled usage will be 0") {
                REQUIRE(usage_resp_msg.key == 1);
                REQUIRE(usage_resp_msg.distance_traveled_um == 0);
            }
        }
        WHEN("passing a position of 0 steps with -100 encoder pulses") {
            handler.handle_message(Ack{.group_id = 11,
                                       .seq_id = 8,
                                       .current_position_steps = 0,
                                       .encoder_position = -1000,
                                       .position_flags = 0x1,
                                       .ack_id = AckMessageId::timeout,
                                       .start_encoder_position = 0,
                                       .usage_key = 1});
            CHECK(mcc.queue.size() == 1);
            CHECK(muc.queue.size() == 1);

            auto resp = mcc.queue.front();
            auto resp_msg = std::get<can::messages::MoveCompleted>(resp.second);
            THEN("the non-position values should be passed through") {
                REQUIRE(resp_msg.group_id == 11);
                REQUIRE(resp_msg.seq_id == 8);
                REQUIRE(resp_msg.ack_id == 3);
                REQUIRE(resp_msg.position_flags == 0x1);
            }
            THEN("the position value should still be 0") {
                REQUIRE(resp_msg.current_position_um == 0);
            }
            THEN("the encoder value should be -500") {
                REQUIRE(resp_msg.encoder_position_um == -500);
            }
            auto usage_resp = muc.queue.front();
            auto usage_resp_msg =
                std::get<usage_messages::IncreaseDistanceUsage>(usage_resp);
            THEN("The distance traveled usage will be populated") {
                REQUIRE(usage_resp_msg.key == 1);
                REQUIRE(usage_resp_msg.distance_traveled_um == 500);
            }
        }
        WHEN("passing a position of fullscale steps") {
            handler.handle_message(
                Ack{.group_id = 0,
                    .seq_id = 0,
                    .current_position_steps = UINT_MAX,
                    .ack_id = AckMessageId::complete_without_condition,
                    .start_encoder_position = 0,
                    .usage_key = 1});
            CHECK(mcc.queue.size() == 1);
            CHECK(muc.queue.size() == 1);
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
            CHECK(muc.queue.size() == 1);
            auto resp = mcc.queue.front();
            auto resp_msg = std::get<can::messages::MoveCompleted>(resp.second);
            THEN("the position in micrometers should be accurate") {
                REQUIRE(resp_msg.current_position_um == 3125);
            }
        }
    }
}
