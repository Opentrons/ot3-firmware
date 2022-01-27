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
                REQUIRE(tools.gripper ==
                        presence_sensing_driver::ToolType::GRIPPER);
            }
        }
    }
}