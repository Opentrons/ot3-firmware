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
        2335 - PIPETTE_384_CHAN_Z_CARRIER_DETECTION_UPPER_BOUND
        3548 - PIPETTE_MULTI_A_CARRIER_DETECTION_UPPER_BOUND
        1778 - PIPETTE_96_CHAN_Z_CARRIER_DETECTION_UPPER_BOUND
        569 - PIPETTE_SINGLE_Z_CARRIER_DETECTION_UPPER_BOUND
        1171 - PIPETTE_MULTI_Z_CARRIER_DETECTION_UPPER_BOUND
        2 - NOTHING_CONNECTED_Z_CARRIER_UPPER_BOUND
        20 - NOTHING_CONNECTED_A_CARRIER
        */

        WHEN("get_tool func is called and raw ADC readings are within bounds") {
            auto adc_comms = adc::MockADC(1855, 2850, 666);
            auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);
            attached_tools::AttachedTools tools;
            bool updated;
            std::tie(updated, tools) = ps.update_tools();
            THEN("Tools mapped to voltage reading") {
                REQUIRE(updated);
                REQUIRE(tools.gripper == can::ids::ToolType::gripper);
                REQUIRE(tools.a_motor == can::ids::ToolType::pipette_multi_chan);
                REQUIRE(tools.z_motor == can::ids::ToolType::pipette_384_chan);
                AND_WHEN("Tools switch to invalid voltages") {
                    adc_comms.get_z_channel().mock_set_reading_by_voltage(3300);
                    std::tie(updated, tools) = ps.update_tools();
                    THEN("The invalid reading is ignored") {
                        REQUIRE(!updated);
                        REQUIRE(tools.z_motor ==
                                can::ids::ToolType::pipette_384_chan);
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
                    adc_comms.get_a_channel().mock_set_reading_by_voltage(2850);
                    std::tie(updated, tools) = ps.update_tools();
                    THEN("the valid tool is read out") {
                        REQUIRE(updated);
                        REQUIRE(tools.a_motor ==
                                can::ids::ToolType::pipette_multi_chan);
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
