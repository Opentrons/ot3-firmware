#include "catch2/catch.hpp"
#include "presence-sensing/core/presence_sensing_driver.hpp"
#include "presence-sensing/tests/mock_adc.hpp"

SCENARIO("get readings called on ADC") {
    GIVEN("adc_comms instance") {
        static auto adc_comms = adc::MockADC{};
        WHEN("get_readings called on adc_comms") {
            static auto adc_readings = adc_comms.get_readings();
            THEN("readings rcved") { REQUIRE(adc_readings.gripper == 666); }
        }
    }
}