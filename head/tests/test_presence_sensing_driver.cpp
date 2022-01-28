#include "catch2/catch.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/tests/mock_adc.hpp"

SCENARIO("get readings called on presence sensing driver") {
    GIVEN("PresenceSensingDriver instance") {
        auto raw_readings =
            adc::RawADCReadings{.z_motor = 666, .a_motor = 666, .gripper = 666};
        static auto adc_comms = adc::MockADC{raw_readings};

        auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);

        WHEN("get_readings func is called") {
            auto voltage_readings = ps.get_readings();

            THEN("mocked driver readings read") {
                REQUIRE(voltage_readings.gripper == 536);
            }
        }
    }
}

SCENARIO("get_tool called on presence sensing driver") {
    GIVEN("PresenceSensingDriver instance") {
        auto raw_readings =
            adc::RawADCReadings{.z_motor = 666, .a_motor = 666, .gripper = 666};
        static auto adc_comms = adc::MockADC{raw_readings};

        auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);

        WHEN("get_tool func is called") {
            auto tools = ps.get_tool(ps.get_readings());

            THEN("Tools mapped to voltage reading") {
                REQUIRE(tools.gripper == presence_sensing_driver::GRIPPER);
            }
        }
    }
}

SCENARIO("ot3 tools list for tool detection") {
    GIVEN("ot3 tools lower and upper voltage bounds for tool detection") {
        auto tools = presence_sensing_driver::OT3ToolList;
        WHEN("when lower bound compared with upper bound") {
            THEN("lower bound is always found lower or equal to upper bounds") {
                for (auto& element : tools) {
                    REQUIRE(element.detection_lower_bound <=
                            element.detection_upper_bound);
                }
            }
        }
    }
}