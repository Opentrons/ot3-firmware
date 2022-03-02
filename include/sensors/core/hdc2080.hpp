#pragma once

#include "can/core/ids.hpp"
// TODO (lc 02-16-2022) We should refactor the fixed point
// helper functions such that they live in a shared location.
#include "motor-control/core/utils.hpp"

/*
 * Configurations and register information for the TI HDC2080 sensor
 *
 * Datasheet can be found at:
 * https://www.ti.com/lit/ds/symlink/hdc2080.pdf?ts=1644850101397&ref_url=https%253A%252F%252Fwww.google.com%252F#:~:text=The%20HDC2080%20device%20is%20an,to%20dissipate%20condensation%20and%20moisture.
 */

namespace hdc2080_utils {

// constants
constexpr float TEMP_CONST_MULTIPLIER = 165.0;
constexpr float TEMP_CONST = 40.5;
constexpr float HUMIDITY_CONST = 100.0;

// max bits for regular sensor value reading is 2^16
constexpr float MAX_SIZE = 65536.0;

constexpr uint16_t ADDRESS = 0x41 << 1;
constexpr uint16_t DEVICE_ID = 0x07D0;

// Registers to read from or write to
constexpr uint8_t INTERRUPT_REGISTER = 0x07;
constexpr uint8_t DRDY_CONFIG = 0x0E;
constexpr uint8_t MEASURE_REGISTER = 0x0F;
constexpr uint8_t DEVICE_ID_REGISTER = 0xFE;
// Low configurations for both temperature and humidity
// this records the status when a reading goes below
// a certain threshold.
constexpr uint8_t LSB_TEMPERATURE_REGISTER = 0x00;
constexpr uint8_t LSB_HUMIDITY_REGISTER = 0x02;

constexpr uint8_t MSB_HUMIDITY_REGISTER = 0x01;
constexpr uint8_t MSB_TEMPERATURE_REGISTER = 0x03;

// humidity sensor configurations
constexpr uint8_t SAMPLE_RATE =
    (5 << 4) | (1 << 2) | (1 << 1);  // 1 sample/second, positive DRDY output
constexpr uint8_t SET_DATARDY = 1 << 7;
constexpr uint8_t BEGIN_MEASUREMENT_RECORDING = 1;

inline auto convert(uint16_t data, can_ids::SensorType type)
    -> sq14_15 {
    switch (type) {
        case can_ids::SensorType::humidity: {
            // returns humidity in relative humidity percentage
            float calculated_humidity =
                HUMIDITY_CONST * ((float)data / MAX_SIZE);
            return convert_to_fixed_point(calculated_humidity, 15);
        }
        case can_ids::SensorType::temperature: {
            // returns temperature in celsius
            float calculated_temp =
                TEMP_CONST_MULTIPLIER * ((float)data / MAX_SIZE) - TEMP_CONST;
            return convert_to_fixed_point(calculated_temp, 15);
        }
        default:
            return 0.0;
    }
}

}  // namespace hdc2080_utils
