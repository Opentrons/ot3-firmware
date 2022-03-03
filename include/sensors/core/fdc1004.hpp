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

// CHA
constexpr uint16_t POSITIVE_INPUT_CHANNEL = 0x0;
// CHB
constexpr uint16_t NEGATIVE_INPUT_CHANNEL = 0x4 << 10;
// configurations
constexpr uint16_t DEVICE_CONFIGURATION =
    0x0 << 13 |                             // CHA = CIN1 (U.FL Connector)
    0x4 << 10;                              // CHB = CAPDAC
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

inline auto convert_capacitance(uint32_t capacitance, uint16_t read_count,
                                float current_offset) -> sq14_15 {
    auto average =
        static_cast<float>(capacitance) / static_cast<float>(read_count);
    float converted_capacitance = average / MAX_MEASUREMENT + current_offset;
    return convert_to_fixed_point(converted_capacitance, 15);
}

inline auto update_capdac(uint16_t capacitance, float current_offset)
    -> uint16_t {
    /**
     * CAPDAC is a unitless offset. To calculate the capacitive offset,
     * you would multiply CAPDAC (unitless) * CAPDAC_OFFSET (pF)
     */
    float capdac =
        (static_cast<float>(capacitance) + current_offset) / CAPDAC_OFFSET;
    if (capdac > MAX_CAPDAC_OFFSET) {
        capdac = MAX_CAPDAC_OFFSET;
    }
    uint16_t new_capdac = static_cast<uint8_t>(capdac) << MAX_CAPDAC_RESOLUTION;

    return new_capdac;
}

inline auto get_offset_pf(uint16_t unitless_capdac) -> float {
    return static_cast<float>(unitless_capdac) * CAPDAC_OFFSET;
}

}  // namespace fdc1004_utils
