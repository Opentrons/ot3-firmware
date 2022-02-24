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
            static auto adc_comms = adc::MockADC(2332, 3548, 666);
            auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);
            auto tools = attached_tools::AttachedTools(
                ps.get_readings(), tool_detection::lookup_table());

            THEN("Tools mapped to voltage reading") {
                REQUIRE(tools.gripper == can_ids::ToolType::gripper);
                REQUIRE(tools.a_motor == can_ids::ToolType::pipette_multi_chan);
                REQUIRE(tools.z_motor == can_ids::ToolType::pipette_384_chan);
            }
        }
        WHEN("get_tool func is called and raw ADC readings are out of bounds") {
            static auto adc_comms = adc::MockADC(9999, 9999, 9999);
            auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);
            auto tools = attached_tools::AttachedTools(
                ps.get_readings(), tool_detection::lookup_table());

            THEN("Tools mapped to voltage reading") {
                REQUIRE(tools.gripper == can_ids::ToolType::undefined_tool);
                REQUIRE(tools.a_motor == can_ids::ToolType::undefined_tool);
                REQUIRE(tools.z_motor == can_ids::ToolType::undefined_tool);
            }
        }
        WHEN(
            "get_tool func is called and raw ADC readings are for no tool "
            "attached") {
            static auto adc_comms = adc::MockADC(2, 20, 20);
            auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);
            auto tools = attached_tools::AttachedTools(
                ps.get_readings(), tool_detection::lookup_table());

            THEN("Tools mapped to voltage reading") {
                REQUIRE(tools.gripper == can_ids::ToolType::nothing_attached);
                REQUIRE(tools.a_motor == can_ids::ToolType::nothing_attached);
                REQUIRE(tools.z_motor == can_ids::ToolType::nothing_attached);
            }
        }
    }
}
