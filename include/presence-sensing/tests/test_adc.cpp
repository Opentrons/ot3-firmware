#include "catch2/catch.hpp"
#include "common/simulation/adc.hpp"
#include "common/core/adc.hpp"

using namespace adc;

SCENARIO("get adc readings") {
    GIVEN("adc dummy readings") {
        auto subject = sim_adc::SimADC{};
        auto readings = subject.get_readings();
        readings.z_motor = 666;
        readings.a_motor = 666;
        readings.gripper = 666;
    WHEN("adc read") {
        THEN("the readings should match dummy values") {
            REQUIRE()
        }
    }


    }
}