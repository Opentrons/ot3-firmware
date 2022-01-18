#include "catch2/catch.hpp"
#include "presence-sensing/core/presence_sensing_driver.hpp"
#include "presence-sensing/tests/mock_adc.hpp"

SCENARIO("get readings called on presence sensing driver") {
    GIVEN("PresenceSensingDriver instance") {
        static auto adc_comms = adc::MockADC{};

        auto ps = presence_sensing_driver::PresenceSensingDriver(adc_comms);

        WHEN("get_readings func is called") {
            auto adc_readings = ps.get_readings();

            THEN("mocked driver readings read") {
                REQUIRE(adc_readings.gripper == 666);
            }
        }
    }
}