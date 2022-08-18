#include <tuple>

#include "catch2/catch.hpp"
#include "common/core/tool_detection.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/tests/mock_adc.hpp"

SCENARIO("get_tool called on presence sensing driver") {
    GIVEN(
        "PresenceSensingDriver instance with valid and invalid ADC readings") {
        /*
        Within bounds valid raw ADC readings
        580 - PIPETTE_Z_CARRIER_DETECTION_UPPER_BOUND
        5 - NOTHING_CONNECTED_Z_CARRIER_UPPER_BOUND
        50 - NOTHING_CONNECTED_A_CARRIER_UPPER_BOUND
        */

        WHEN("get_tool func is called and raw ADC readings are within bounds") {
            auto adc_comms = adc::MockADC(570, 2950, 666);
            auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);
            attached_tools::AttachedTools tools;
            bool updated;
            std::tie(updated, tools) = ps.update_tools();
            THEN("Tools mapped to voltage reading") {
                REQUIRE(updated);
                REQUIRE(tools.gripper == can::ids::ToolType::gripper);
                REQUIRE(tools.a_motor == can::ids::ToolType::pipette);
                REQUIRE(tools.z_motor == can::ids::ToolType::pipette);
                AND_WHEN("Tools switch to invalid voltages") {
                    adc_comms.get_z_channel().mock_set_reading_by_voltage(3300);
                    std::tie(updated, tools) = ps.update_tools();
                    THEN("The invalid reading is ignored") {
                        REQUIRE(!updated);
                        REQUIRE(tools.z_motor == can::ids::ToolType::pipette);
                    }
                }
            }
        }
        WHEN("get_tool func is called and raw ADC readings are out of bounds") {
            auto adc_comms = adc::MockADC(5000, 4000, 3400);
            auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);
            attached_tools::AttachedTools tools;
            bool updated;
            std::tie(updated, tools) = ps.update_tools();
            THEN("Tools remain invalid mapped to voltage reading") {
                REQUIRE(!updated);
                REQUIRE(tools.gripper == can::ids::ToolType::undefined_tool);
                REQUIRE(tools.a_motor == can::ids::ToolType::undefined_tool);
                REQUIRE(tools.z_motor == can::ids::ToolType::undefined_tool);
                AND_WHEN("tools switch to valid voltages") {
                    adc_comms.get_a_channel().mock_set_reading_by_voltage(2950);
                    std::tie(updated, tools) = ps.update_tools();
                    THEN("the valid tool is read out") {
                        REQUIRE(updated);
                        REQUIRE(tools.a_motor == can::ids::ToolType::pipette);
                        REQUIRE(tools.z_motor ==
                                can::ids::ToolType::undefined_tool);
                        REQUIRE(tools.gripper ==
                                can::ids::ToolType::undefined_tool);
                    }
                }
            }
        }
        WHEN(
            "get_tool func is called and raw ADC readings are for no tool "
            "attached") {
            static auto adc_comms = adc::MockADC(2, 20, 20);
            auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);
            attached_tools::AttachedTools tools;
            bool updated;
            std::tie(updated, tools) = ps.update_tools();

            THEN("Tools mapped to voltage reading") {
                REQUIRE(updated);
                REQUIRE(tools.gripper == can::ids::ToolType::nothing_attached);
                REQUIRE(tools.a_motor == can::ids::ToolType::nothing_attached);
                REQUIRE(tools.z_motor == can::ids::ToolType::nothing_attached);
            }
        }
    }
}
