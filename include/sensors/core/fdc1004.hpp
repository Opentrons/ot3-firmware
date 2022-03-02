#pragma once

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

namespace fdc1004_utils {

// Capacitance Sensor Address and Registers
constexpr uint16_t ADDRESS = 0x50 << 1;
constexpr uint8_t MSB_MEASUREMENT_1 = 0x00;
constexpr uint8_t LSB_MEASUREMENT_1 = 0x01;
constexpr uint8_t CONFIGURATION_MEASUREMENT = 0x08;
constexpr uint8_t FDC_CONFIGURATION = 0x0C;
constexpr uint8_t DEVICE_ID_REGISTER = 0xFF;

// configurations
constexpr uint16_t DEVICE_CONFIGURATION =
    0x0 << 13 |  // CHA = CIN1 (U.FL Connector)
    0x4 << 10;   // CHB = CAPDAC
constexpr uint16_t SAMPLE_RATE = 1 << 10 |  // 100S/s
                                        1 << 8 |   // Repeat enabled
                                        1 << 7;    // Measurement 1 enabled

constexpr uint16_t DEVICE_ID = 0x1004;

// Constants
constexpr int MAX_CAPDAC_RESOLUTION = 5;
constexpr int MSB_SHIFT = 16;
constexpr float MAX_CAPDAC_OFFSET = 0x1F << MAX_CAPDAC_RESOLUTION;
constexpr float MAX_MEASUREMENT = 524288.0;
// single ended capdac offset measurement in pF
constexpr float CAPDAC_OFFSET = 3.125;

inline auto convert(uint32_t measurement,
                    uint16_t read_count,
                    float current_offset) -> sq14_15 {
    auto average =
        static_cast<float>(measurement) / static_cast<float>(read_count);
    float capacitance = average / MAX_MEASUREMENT + current_offset;
    return convert_to_fixed_point(capacitance, 15);
}

inline auto calculate_capdac(uint16_t measurement,
                             uint16_t read_count,
                             float current_offset)
    -> uint16_t {
    float average =
        static_cast<float>(measurement) / static_cast<float>(read_count);
    float offset = (average + current_offset) / CAPDAC_OFFSET;
    if (offset > MAX_CAPDAC_OFFSET) {
        offset = MAX_CAPDAC_OFFSET;
    }
    uint16_t new_offset = static_cast<uint8_t>(offset) << MAX_CAPDAC_RESOLUTION;

    return new_offset;
}

}  // namespace fdc1004_utils
