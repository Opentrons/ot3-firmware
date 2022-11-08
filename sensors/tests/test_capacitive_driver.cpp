#include "catch2/catch.hpp"
#include "common/core/logging.h"
#include "sensors/core/fdc1004.hpp"

SCENARIO("converting data to 24 bit value from IC registers") {
    GIVEN("The max MSB and LSB values of the fdc1004") {
        constexpr int16_t MSB = 0x7fff;
        constexpr int16_t LSB = 0xff00;
        int32_t converted_registers = sensors::fdc1004::convert_reads(MSB, LSB);
        LOG("the converted value: %d", converted_registers);
        REQUIRE(float(converted_registers) ==
                sensors::fdc1004::MAX_RAW_MEASUREMENT);
        int32_t capacitance =
            sensors::fdc1004::convert_capacitance(converted_registers, 1, 0.0);
        THEN("The capacitance should be equal to 15 pF") {
            REQUIRE(capacitance == 15);
        }
    }
    GIVEN("Half the max MSB and LSB values of the fdc1004") {
        constexpr int16_t MSB = 0x3fff;
        constexpr int16_t LSB = 0xff00;
        int32_t converted_registers = sensors::fdc1004::convert_reads(MSB, LSB);
        LOG("the converted value: %d", converted_registers);
        REQUIRE(converted_registers ==
                (int(sensors::fdc1004::MAX_RAW_MEASUREMENT) >> 1));
        float capacitance =
            sensors::fdc1004::convert_capacitance(converted_registers, 1, 0.0);
        float expected = 7.5;
        THEN("The capacitance should be equal to 7.5 pF") {
            REQUIRE(capacitance == Approx(expected).epsilon(1e-4));
        }
    }
}