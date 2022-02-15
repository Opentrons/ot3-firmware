#include "catch2/catch.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/tests/mock_adc.hpp"

SCENARIO("Raw ADC readings to voltage conversions") {
    GIVEN("Raw readings and an ADC instance") {
        static auto adc_comms = adc::MockADC{666, 333, 999};
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
