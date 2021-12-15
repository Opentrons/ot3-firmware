#include "can/core/arbitration_id.hpp"
#include "catch2/catch.hpp"

using namespace can_arbitration_id;
using namespace can_ids;

SCENARIO("Arbitration ID") {

    GIVEN("an arbitration id object") {
        auto subject = ArbitrationId();
        subject.node_id(NodeId::head_l);
        subject.originating_node_id(NodeId::head_r);
        subject.function_code(FunctionCode::bootloader);
        subject.message_id(MessageId::heartbeat_request);


        WHEN("converted to an integer") {
            THEN("the values are read correctly") {
                REQUIRE(subject.node_id() == NodeId::head_l);
                REQUIRE(subject.originating_node_id() == NodeId::head_r);
                REQUIRE(subject.function_code() == FunctionCode::bootloader);
                REQUIRE(subject.message_id() == MessageId::heartbeat_request);
            }
            THEN("the bits are set correctly") {
                REQUIRE(subject.get_id() ==
                        ((static_cast<uint32_t>(subject.message_id())
                          << ArbitrationId::message_id_shift) |
                         (static_cast<uint32_t>(subject.node_id())
                          << ArbitrationId::node_id_shift) |
                         (static_cast<uint32_t>(subject.originating_node_id())
                          << ArbitrationId::originating_node_id_shift) |
                         (static_cast<uint32_t>(subject.function_code())
                          << ArbitrationId::function_code_shift)));
            }
        }

        WHEN("creating another arbitration id from integer") {
            auto subject2 = ArbitrationId(subject.get_id());
            THEN("the individual values are the same") {
                REQUIRE(subject.node_id() == subject2.node_id());
                REQUIRE(subject.function_code() == subject2.function_code());
                REQUIRE(subject.message_id() == subject2.message_id());
            }
        }
    }

    GIVEN("an empty arbitration id object") {
        auto subject = ArbitrationId();
        WHEN("repeatedly setting values") {
            subject.message_id(MessageId::device_info_request);
            subject.message_id(MessageId::get_move_group_request);
            subject.originating_node_id(NodeId::broadcast);
            subject.originating_node_id(NodeId::gantry_x);
            subject.originating_node_id(NodeId::pipette);
            subject.node_id(NodeId::pipette);
            subject.node_id(NodeId::gantry_x);
            subject.node_id(NodeId::broadcast);

            THEN("the final values are written") {
                REQUIRE(subject.message_id() ==
                        MessageId::get_move_group_request);
                REQUIRE(subject.node_id() == NodeId::broadcast);
                REQUIRE(subject.originating_node_id() == NodeId::pipette);
            }
        }
    }
}
