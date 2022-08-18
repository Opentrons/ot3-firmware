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
                       (can::ids::ToolType::undefined_tool),
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
            THEN("the tool and carrier are undefined") {
                REQUIRE(tool_detection::tooltype_from_reading(reading) ==
                        can::ids::ToolType::undefined_tool);
                REQUIRE(tool_detection::carrier_from_reading(reading) ==
                        tool_detection::Carrier::UNKNOWN);
            }
        }
        WHEN("checking holes in the bounds set for tool mounts") {
            auto which_tool = GENERATE(tool_detection::Carrier::A_CARRIER,
                                       tool_detection::Carrier::Z_CARRIER);
            // can't modify the lookup table, gotta copy
            auto filtered = tool_detection::lookup_table_filtered(which_tool);
            auto copied = std::vector(filtered.begin(), filtered.end());
            std::sort(copied.begin(), copied.end(),
                      [](const tool_detection::Tool& a,
                         const tool_detection::Tool& b) {
                          return a.bounds.upper < b.bounds.upper;
                      });
            auto trailer =
                std::ranges::subrange(copied.begin(), --copied.end());
            auto leader = std::ranges::subrange(++copied.begin(), copied.end());
            for (auto trailer_it = trailer.begin(), leader_it = leader.begin();
                 trailer_it != trailer.end() && leader_it != leader.end();
                 trailer_it++, leader_it++) {
                INFO("Checking tool detection for carrier "
                     << static_cast<uint32_t>(which_tool) << ", tool type "
                     << static_cast<uint32_t>(trailer_it->tool_type) << " ("
                     << trailer_it->bounds.lower << "->"
                     << trailer_it->bounds.upper << "mV) -> "
                     << static_cast<uint32_t>(leader_it->tool_type) << " ("
                     << leader_it->bounds.lower << "->"
                     << leader_it->bounds.upper << "mV)");
                // Just above the upper range of the first
                REQUIRE(tool_detection::tooltype_from_reading(
                            trailer_it->bounds.upper, filtered) ==
                        can::ids::ToolType::undefined_tool);
                // Just below the lower range of the second
                REQUIRE(tool_detection::tooltype_from_reading(
                            leader_it->bounds.lower - 1, filtered) ==
                        can::ids::ToolType::undefined_tool);
                // Dead-on in the middle
                auto mid_value =
                    (trailer_it->bounds.upper + leader_it->bounds.lower) / 2;
                INFO("Bounds gap mid value " << mid_value
                                             << " matched to a tool");
                REQUIRE(tool_detection::tooltype_from_reading(mid_value,
                                                              filtered) ==
                        can::ids::ToolType::undefined_tool);
            }
        }
    }
}
