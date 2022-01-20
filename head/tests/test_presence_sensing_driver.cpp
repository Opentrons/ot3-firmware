#include "catch2/catch.hpp"
#include "head/core/presence_sensing_driver.hpp"
#include "head/tests/mock_adc.hpp"

SCENARIO("get readings called on presence sensing driver") {
    GIVEN("PresenceSensingDriver instance") {
        static auto adc_comms = adc::MockADC{};

        auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);

        WHEN("get_readings func is called") {
            auto voltage_readings = ps.get_readings();

            THEN("mocked driver readings read") {
                REQUIRE(voltage_readings.gripper == 536);
            }
        }
    }
}