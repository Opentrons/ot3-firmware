#include "catch2/catch.hpp"
#include "common/core/adc_channel.hpp"
#include "common/tests/mock_adc_channel.hpp"

SCENARIO("ADC channel conversions") {
    GIVEN("an ADC channel with a limited input range and a 1:1 translation") {
        auto chan = adc::MockADCChannel<1000, 1000>(10);
        WHEN("Translating an in-bounds value") {
            chan.mock_set_reading(100);
            THEN("The reading is 1:1") { REQUIRE(chan.get_voltage() == 100); }
        }
        WHEN("Translating out-of-bounds-high values") {
            chan.mock_set_reading(1001);
            THEN("the reading is still 1:1") {
                REQUIRE(chan.get_voltage() == 1001);
            }
        }
    }
}
