#include "catch2/catch.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/tests/mock_adc.hpp"

SCENARIO("Raw ADC readings to voltage conversions") {
    GIVEN("Raw readings and an ADC instance") {
        auto adc_comms = adc::MockADC{666, 333, 999};
        adc_comms.get_z_channel().mock_set_reading(666);
        adc_comms.get_a_channel().mock_set_reading(333);
        adc_comms.get_gripper_channel().mock_set_reading(999);
        WHEN("get_readings function is called") {
            auto voltage_readings = adc_comms.get_voltages();

            THEN("RAW readings converted to voltages") {
                REQUIRE(voltage_readings.gripper == 805);
                REQUIRE(voltage_readings.a_motor == 268);
                REQUIRE(voltage_readings.z_motor == 536);
            }
        }
    }
}
