#include "catch2/catch.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/tests/mock_adc.hpp"

SCENARIO("get readings called on presence sensing driver") {
    GIVEN("PresenceSensingDriver instance") {
        auto raw_readings =
            adc::RawADCReadings{.z_motor = 666, .a_motor = 666, .gripper = 666};
        static auto adc_comms = adc::MockADC{raw_readings};

        auto at = ot3_tool_list::AttachedTool{};

        auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms, at);

        WHEN("get_readings func is called") {
            auto voltage_readings = ps.get_readings();

            THEN("mocked driver readings read") {
                REQUIRE(voltage_readings.gripper == 536);
            }
        }
    }
}

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
            auto raw_readings = adc::RawADCReadings{
                .z_motor = 2332, .a_motor = 3548, .gripper = 666};
            static auto adc_comms = adc::MockADC{raw_readings};
            auto ps = presence_sensing_driver::PresenceSensingDriver(
                adc_comms, ot3_tool_list::AttachedTool{});
            auto tools = ot3_tool_list::AttachedTool(
                ps.get_readings(), ot3_tool_list::get_tool_list());

            THEN("Tools mapped to voltage reading") {
                REQUIRE(tools.gripper == can_ids::ToolType::gripper);
                REQUIRE(tools.a_motor == can_ids::ToolType::pipette_multi_chan);
                REQUIRE(tools.z_motor == can_ids::ToolType::pipette_384_chan);
            }
        }
        WHEN("get_tool func is called and raw ADC readings are out of bounds") {
            auto raw_readings = adc::RawADCReadings{
                .z_motor = 9999, .a_motor = 9999, .gripper = 9999};
            static auto adc_comms = adc::MockADC{raw_readings};
            auto ps = presence_sensing_driver::PresenceSensingDriver(
                adc_comms, ot3_tool_list::AttachedTool{});
            auto tools = ot3_tool_list::AttachedTool(
                ps.get_readings(), ot3_tool_list::get_tool_list());

            THEN("Tools mapped to voltage reading") {
                REQUIRE(tools.gripper == can_ids::ToolType::undefined_tool);
                REQUIRE(tools.a_motor == can_ids::ToolType::undefined_tool);
                REQUIRE(tools.z_motor == can_ids::ToolType::undefined_tool);
            }
        }
        WHEN(
            "get_tool func is called and raw ADC readings are for no tool "
            "attached") {
            auto raw_readings =
                adc::RawADCReadings{.z_motor = 2, .a_motor = 20, .gripper = 20};
            static auto adc_comms = adc::MockADC{raw_readings};
            auto ps = presence_sensing_driver::PresenceSensingDriver(
                adc_comms, ot3_tool_list::AttachedTool{});
            auto tools = ot3_tool_list::AttachedTool(
                ps.get_readings(), ot3_tool_list::get_tool_list());

            THEN("Tools mapped to voltage reading") {
                REQUIRE(tools.gripper == can_ids::ToolType::nothing_attached);
                REQUIRE(tools.a_motor == can_ids::ToolType::nothing_attached);
                REQUIRE(tools.z_motor == can_ids::ToolType::nothing_attached);
            }
        }
    }
}

SCENARIO("ot3 tools list for tool detection") {
    GIVEN("ot3 tools lower and upper voltage bounds for tool detection") {
        auto tools = ot3_tool_list::get_tool_list();
        WHEN("when lower bound compared with upper bound") {
            THEN("lower bound is always found lower or equal to upper bounds") {
                for (auto& element : tools) {
                    REQUIRE(element.bounds.lower <= element.bounds.upper);
                }
            }
        }
    }
}