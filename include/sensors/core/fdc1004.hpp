#pragma once

#include <cstdlib>
#include <limits>

#include "common/core/logging.h"

// TODO (lc 02-16-2022) We should refactor the fixed point
// helper functions such that they live in a shared location.
#include "motor-control/core/utils.hpp"

/*
 * FDC1004 Capacitive Sensor
 *
 * Datasheet:
 * https://www.ti.com/lit/ds/symlink/fdc1004.pdf?ts=1645556558559&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FFDC1004
 *
 * The configuration of the sensor is always in single-ended measurement mode.
 * We are currently only using 1 out of the 4 measurement channels.
 *
 * By default, CAPDAC is enabled. The max resolution of the capdac is 5 bits.
 *
 * Capacitance measurement is a total of 24 bits long. You must do consecutive
 * reads of the MSB and LSB registers to get the data.
 */

namespace sensors {
namespace fdc1004 {

// Capacitance Sensor Address and Registers
constexpr uint16_t ADDRESS = 0x50 << 1;
constexpr uint8_t MSB_MEASUREMENT_1 = 0x00;
constexpr uint8_t LSB_MEASUREMENT_1 = 0x01;
constexpr uint8_t CONFIGURATION_MEASUREMENT = 0x08;
constexpr uint8_t FDC_CONFIGURATION = 0x0C;
constexpr uint8_t DEVICE_ID_REGISTER = 0xFF;

// CHA
constexpr uint16_t POSITIVE_INPUT_CHANNEL = 0x0;
// CHB
constexpr uint16_t NEGATIVE_INPUT_CHANNEL = 0x4 << 10;
// configurations: our reads will be differential reads of
// the U.FL connector on one channel and the internal
// common-mode compensator, CAPDAC, on the other.
constexpr uint16_t DEVICE_CONFIGURATION =
    0x0 << 13 |  // CHA = CIN1 (U.FL Connector)
    0x4 << 10;   // CHB = CAPDAC
constexpr uint8_t DEVICE_CONFIGURATION_MSB =
    static_cast<uint8_t>(DEVICE_CONFIGURATION >> 8);
constexpr uint8_t DEVICE_CONFIGURATION_LSB =
    static_cast<uint8_t>(DEVICE_CONFIGURATION & 0xff);
constexpr uint16_t SAMPLE_RATE = 1 << 10 |  // 100S/s
                                 1 << 8 |   // Repeat enabled
                                 1 << 7;    // Measurement 1 enabled

constexpr uint8_t SAMPLE_RATE_MSB = static_cast<uint8_t>(SAMPLE_RATE >> 8);
constexpr uint8_t SAMPLE_RATE_LSB = static_cast<uint8_t>(SAMPLE_RATE & 0xff);
constexpr uint16_t DEVICE_ID = 0x1004;

// Constants. The capdac is a synthetic comparison source intended to
// eliminate common-mode values in the differential capacitance measurements.
// The sensor has a narrow +-15pF measurement range, but that's on top
// of the capdac - the capdac is "subtracted", more or less, before the
// capacitance is read.
constexpr float CAPDAC_PF_PER_LSB = 3.125;
constexpr std::size_t CAPDAC_BITS = 5;
constexpr uint8_t MAX_CAPDAC_RAW_VALUE = (1 << CAPDAC_BITS) - 1;
constexpr float MAX_CAPDAC_PF =
    static_cast<float>(MAX_CAPDAC_RAW_VALUE) * CAPDAC_PF_PER_LSB;

// Our +-15pF comes in 24 bits, left shifted across two 16 bit registers.
constexpr std::size_t CONVERSION_BITS = 24;
constexpr float MAX_MEASUREMENT_PF = 15;
constexpr float MAX_RAW_MEASUREMENT =
    float(std::numeric_limits<int32_t>::max() >>
          (sizeof(uint32_t) * 8) - CONVERSION_BITS);
//    float(std::numeric_limits<int32_t>::max() >> 8);
//          ((sizeof(uint32_t) * 8) - CONVERSION_BITS));

// Because we're doing big gantry moves, we'll probably have to handle
// the parasitic capacitance of the system shifting around a lot. We'll
// need to automatically reset our capdac to account for changing
// gross-scale capacitance conditions, and we'll do it by bumping up
// the capdac every time we get half way to either edge of our range.
constexpr float CAPDAC_REZERO_THRESHOLD_PF = CAPDAC_PF_PER_LSB / 2;

// Convert an accumulated raw reading, the number of reads that were
// accumulated, and the current offset and turn it into a value in pF.
inline auto convert_capacitance(int32_t capacitance_accumulated_raw,
                                uint16_t read_count, float current_offset_pf)
    -> float {
    auto average = static_cast<float>(capacitance_accumulated_raw) /
                   static_cast<float>(read_count);
    float converted_capacitance =
        (average / MAX_RAW_MEASUREMENT) * MAX_MEASUREMENT_PF;
    LOG("Conversion: max raw %f mmt %f direct conversion %f offset %f",
        MAX_RAW_MEASUREMENT, average, converted_capacitance, current_offset_pf);
    return converted_capacitance + current_offset_pf;
}

// Take the two buffers and turn them into a 24 bit raw measurement.
inline auto convert_reads(uint16_t msb, uint16_t lsb) -> int32_t {
    // measurements are presented in a 16 bit most significant register
    // and a 16 bit least significant register. Data is left-aligned;
    // the most significant register has 16 valid bits, and the least
    // significant register has 8 valid bits in the MSB.
    // The data is also signed. That means that we first want to
    // formulate it as a 32 bit unsigned int to use logical shifts;
    // then take those 32 bits and interpret them as a signed 32 bit
    // integer, and arithmetic right-shift back to 24 bits.

    return (static_cast<int32_t>((static_cast<uint32_t>(msb) << 16) |
                                 static_cast<uint32_t>(lsb)) >>
            8);
}

// Turn a capacitance value into the value to send to the capdac
// control register to use that offset.
inline auto get_capdac_raw(float offset_pf) -> uint8_t {
    auto capdac = static_cast<uint8_t>(offset_pf / CAPDAC_PF_PER_LSB);
    return ((capdac > MAX_CAPDAC_RAW_VALUE) ? MAX_CAPDAC_RAW_VALUE : capdac);
}

// Check the current absolute reading (i.e. after the offset is applied)
// and the offset that was applied and, if necessary, generate a new
// appropriately-capped offset value that is ready for sending to the
// sensor.
inline auto update_offset(float capacitance_pf, float current_offset_pf)
    -> float {
    if (std::abs(capacitance_pf - current_offset_pf) <
        CAPDAC_REZERO_THRESHOLD_PF) {
        // we haven't gotten close enough to the edge of our range to
        // rezero
        return current_offset_pf;
    }
    LOG("Capacitance %f needs rezeroing (%f-%f=%f)", capacitance_pf,
        capacitance_pf, current_offset_pf, capacitance_pf - current_offset_pf);

    // we're halfway to the edge of our range; let's try and rezero so
    // that the current reading is in the center
    uint8_t capdac = get_capdac_raw(capacitance_pf);

    return static_cast<float>(capdac) * CAPDAC_PF_PER_LSB;
}

inline constexpr auto device_configuration_msb(uint8_t capdac_raw) -> uint8_t {
    return (DEVICE_CONFIGURATION_MSB | ((capdac_raw >> 3) & 0x7));
}

inline constexpr auto device_configuration_lsb(uint8_t capdac_raw) -> uint8_t {
    return (DEVICE_CONFIGURATION_LSB | ((capdac_raw << 5) & 0xff));
}

};  // namespace fdc1004
};  // namespace sensors
