#include <array>
#include <variant>

#include "can/core/messages.hpp"
#include "catch2/catch.hpp"
#include "motor-control/core/motion_group.hpp"

bool operator==(const can_messages::AddLinearMoveRequest& a,
                const can_messages::AddLinearMoveRequest& b) {
    return a.duration == b.duration && a.seq_id == b.seq_id &&
           a.acceleration == b.acceleration && a.group_id == b.group_id &&
           a.velocity == b.velocity;
}

SCENARIO("Testing a move group") {
    auto group = move_group::MoveGroup<5, can_messages::AddLinearMoveRequest>{};

    GIVEN("a move group") {
        WHEN("is created") {
            THEN("it is initially empty") { REQUIRE(group.empty()); }
            THEN("the size is 0") { REQUIRE(group.size() == 0); }
        }

        WHEN("set_move is called") {
            auto linear_move =
                can_messages::AddLinearMoveRequest{.group_id = 0,
                                                   .seq_id = 1,
                                                   .duration = 2,
                                                   .acceleration = 3,
                                                   .velocity = 4};
            auto result = group.set_move(linear_move);
            auto read_group = group.get_move(linear_move.seq_id);

            THEN("result is true.") { REQUIRE(result == true); }
            THEN("the stored type is correct.") {
                REQUIRE(
                    std::holds_alternative<can_messages::AddLinearMoveRequest>(
                        read_group));
            }
            THEN("the object is stored correctly.") {
                REQUIRE(std::get<can_messages::AddLinearMoveRequest>(
                            read_group) == linear_move);
            }
            THEN("it is not empty") { REQUIRE(!group.empty()); }
            THEN("the size is 1") { REQUIRE(group.size() == 1); }
        }

        WHEN("set_move is called with too high a seq_id") {
            auto linear_move =
                can_messages::AddLinearMoveRequest{.group_id = 0,
                                                   .seq_id = 123,
                                                   .duration = 2,
                                                   .acceleration = 3,
                                                   .velocity = 4};
            auto result = group.set_move(linear_move);
            THEN("result is false.") { REQUIRE(result == false); }
        }

        WHEN("clear is called") {
            auto linear_move =
                can_messages::AddLinearMoveRequest{.group_id = 0,
                                                   .seq_id = 1,
                                                   .duration = 2,
                                                   .acceleration = 3,
                                                   .velocity = 4};
            group.set_move(linear_move);
            group.clear();

            THEN("it is empty") { REQUIRE(group.empty()); }
            THEN("the size is 0") { REQUIRE(group.size() == 0); }
        }
    }

    GIVEN("a move group with multiple moves") {
        auto moves =
            std::array{can_messages::AddLinearMoveRequest{.group_id = 0,
                                                          .seq_id = 0,
                                                          .duration = 100,
                                                          .acceleration = 3,
                                                          .velocity = 4},
                       can_messages::AddLinearMoveRequest{.group_id = 0,
                                                          .seq_id = 1,
                                                          .duration = 200,
                                                          .acceleration = 3,
                                                          .velocity = 4}};
        for (can_messages::AddLinearMoveRequest m : moves) {
            group.set_move(m);
        }
        WHEN("get duration is called") {
            THEN("it is correct") { REQUIRE(group.get_duration() == 300); }
        }
    }
}
