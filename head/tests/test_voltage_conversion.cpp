#include "catch2/catch.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/tests/mock_adc.hpp"

SCENARIO("Raw ADC readings to volate conversions") {
    GIVEN("Raw readings and a PresenceSensingDriver isntance") {
        auto raw_readings =
            adc::RawADCReadings{.z_motor = 666, .a_motor = 666, .gripper = 666};
        static auto adc_comms = adc::MockADC{raw_readings};

        auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);

        WHEN("get_readings func is called") {
            auto voltage_readings = ps.get_readings();

            THEN("RAW readings converted to voltages") {
                REQUIRE(voltage_readings.gripper == 536);
                REQUIRE(voltage_readings.a_motor == 536);
                REQUIRE(voltage_readings.z_motor == 536);
            }
        }
    }
}