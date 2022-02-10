#pragma once

#include "can/core/ids.hpp"

/*
 * Configurations and register information for the TI HDC2080 sensor
 */

namespace humidity_utils {

static auto convert(uint16_t data, can_ids::SensorType type) -> float;

// constants
static constexpr float TEMP_CONST_MULTIPLIER = 165.0;
static constexpr float TEMP_CONST = 40.5;
static constexpr float HUMIDITY_CONST = 100.0;
// max bits on this sensor is 2^8
static constexpr float MAX_SIZE = 65536.0;

static constexpr uint16_t ADDRESS = 0x41 << 1;
static constexpr uint16_t DEVICE_ID = 0x07D0;

// Registers to read from
static constexpr uint8_t INTERRUPT_REGISTER = 0x07;
static constexpr uint8_t DRDY_CONFIG = 0x0E;
// Low configurations for both temperature and humidity
// this records the status when a reading goes below
// a certain threshold.
static constexpr uint8_t TEMPERATURE_REGISTER = 0x00;
static constexpr uint8_t HUMIDITY_REGISTER = 0x02;

// humidity sensor configurations
static constexpr uint8_t SAMPLE_RATE =
    (5 << 4) | (1 << 2) | (1 << 1);  // 1 sample/second, positive DRDY output
static constexpr uint8_t SET_DATARDY = 1 << 7;
static constexpr uint8_t BEGIN_MEASUREMENT_RECORDING = 1;

static auto convert(uint16_t data, can_ids::SensorType type) -> float {
    switch (type) {
        case can_ids::SensorType::humidity: {
            // returns humidity in
            return HUMIDITY_CONST * ((float)data / MAX_SIZE);
        }
        case can_ids::SensorType::temperature: {
            // returns temperature in celsius
            return TEMP_CONST_MULTIPLIER * ((float)data / MAX_SIZE) -
                   TEMP_CONST;
        }
        default:
            return 0.0;
    }
}

}  // namespace humidity_utils
