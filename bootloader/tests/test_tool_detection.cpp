#include "bootloader/core/node_id.h"
#include "common/core/tool_detection.hpp"

SCENARIO("Testing bootloader and application mount ID consistency") {
    GIVEN("The boundaries of valid mount voltages for Z") {
        auto z_mounts = tool_detection::lookup_table_filtered(
            tool_detection::Carrier::Z_CARRIER);
        WHEN("testing the values for each mount") {
            for (auto tool : z_mounts) {
                THEN("the lower bound inclusive value matches") {
                    auto to_check = tool.bounds.lower;
                    REQUIRE(determine_pipette_node_id(to_check) ==
                            can_nodeid_pipette_left_bootloader);
                }
                THEN("the upper bound under 1 matches") {
                    auto to_check = tool.bounds.upper - 1;
                    REQUIRE(determine_pipette_node_id(to_check) ==
                            can_nodeid_pipette_left_bootloader);
                }
                THEN("the midpoint value matches") {
                    auto to_check = (tool.bounds.upper + tool.bounds.lower) / 2;
                    REQUIRE(determine_pipette_node_id(to_check) ==
                            can_nodeid_pipette_left_bootloader);
                }
            }
        }
    }
    GIVEN("The boundaries of valid mount voltages for A") {
        auto a_mounts = tool_detection::lookup_table_filtered(
            tool_detection::Carrier::A_CARRIER);
        WHEN("testing the values for each mount") {
            for (auto tool : a_mounts) {
                THEN("the lower bound inclusive value matches") {
                    auto to_check = tool.bounds.lower;
                    REQUIRE(determine_pipette_node_id(to_check) ==
                            can_nodeid_pipette_right_bootloader);
                }
                THEN("the upper bound under 1 matches") {
                    auto to_check = tool.bounds.upper - 1;
                    REQUIRE(determine_pipette_node_id(to_check) ==
                            can_nodeid_pipette_right_bootloader);
                }
                THEN("the midpoint value matches") {
                    auto to_check = (tool.bounds.upper + tool.bounds.lower) / 2;
                    REQUIRE(determine_pipette_node_id(to_check) ==
                            can_nodeid_pipette_right_bootloader);
                }
            }
        }
    }
    GIVEN("Arbitrary mount voltages matching neither A nor Z") {
        auto values_to_check = GENERATE(3250, 10, 3000, 900);
        WHEN("testing the values") {
            auto left_check = tool_detection::lookup_table_filtered(
                tool_detection::Carrier::Z_CARRIER);
            auto right_check = tool_detection::lookup_table_filtered(
                tool_detection::Carrier::A_CARRIER);
            // Make sure this is still a hole
            CHECK(tool_detection::carrier_from_reading(values_to_check,
                                                       left_check) ==
                  tool_detection::Carrier::UNKOWN);
            CHECK(tool_detection::carrier_from_reading(values_to_check,
                                                       right_check) ==
                  tool_detection::Carrier::UNKOWN);
            THEN("the node id should default to left") {
                REQUIRE(determine_pipette_node_id(values_to_check) ==
                        can_nodeid_pipette_left_bootloader);
            }
        }
    }
}
