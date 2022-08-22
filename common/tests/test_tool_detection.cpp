#include <algorithm>
#include <ranges>
#include <vector>

#include "can/core/ids.hpp"
#include "catch2/catch.hpp"
#include "common/core/tool_detection.hpp"

// Filter tests - no BDD here because these macros don't have
// BDD equivalents
TEMPLATE_TEST_CASE_SIG("tool list by Tool Type type", "",
                       ((can::ids::ToolType TT), TT),
                       (can::ids::ToolType::pipette),
                       (can::ids::ToolType::tool_error),
                       (can::ids::ToolType::gripper),
                       (can::ids::ToolType::nothing_attached)) {
    SECTION("lookup table filtering") {
        auto tool_only = tool_detection::lookup_table_filtered(TT);
        bool checked_anything = false;
        for (auto tool_elem : tool_only) {
            checked_anything = true;
            REQUIRE(tool_elem.tool_type == TT);
        }
        CHECK(checked_anything);
    }
}

TEMPLATE_TEST_CASE_SIG("tool list by carrier", "",
                       ((tool_detection::Carrier C), C),
                       (tool_detection::Carrier::GRIPPER_CARRIER),
                       (tool_detection::Carrier::A_CARRIER),
                       (tool_detection::Carrier::Z_CARRIER)) {
    auto carrier_only = tool_detection::lookup_table_filtered(C);
    bool checked_anything = false;
    for (auto tool_elem : carrier_only) {
        checked_anything = true;
        REQUIRE(tool_elem.tool_carrier == C);
    }
    CHECK(checked_anything);
}

SCENARIO("ot3 tools list for tool detection") {
    GIVEN("ot3 tools lower and upper voltage bounds for tool detection") {
        auto tools = tool_detection::lookup_table();
        WHEN("when lower bound compared with upper bound") {
            THEN("lower bound is always found lower or equal to upper bounds") {
                for (auto& element : tools) {
                    REQUIRE(element.bounds.lower <= element.bounds.upper);
                }
            }
        }
        WHEN("you pass in an out-of-bounds read") {
            uint16_t reading = 3200;
            THEN("the tool and carrier are errored and unknown") {
                REQUIRE(tool_detection::tooltype_from_reading(reading) ==
                        can::ids::ToolType::tool_error);
                auto value = tool_detection::carrier_from_reading(reading);
                REQUIRE(value == tool_detection::Carrier::UNKNOWN);
            }
        }
    }
}
