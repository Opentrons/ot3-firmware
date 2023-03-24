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

/ Capacitance Sensor Address and Registers
constexpr uint16_t ADDRESS = 0x50 << 1;

enum class Registers : uint8_t {
    MEAS1_MSB = 0x0,
    MEAS1_LSB = 0x01,
    MEAS2_MSB = 0x02,
    MEAS2_LSB = 0x03,
    MEAS3_MSB = 0x04,
    MEAS3_LSB = 0x05,
    MEAS4_MSB = 0x06,
    MEAS4_LSB = 0x07,
    CONF_MEAS1 = 0x08,
    CONF_MEAS2 = 0x09,
    CONF_MEAS3 = 0x0A,
    CONF_MEAS4 = 0x0B,
    FDC_CONF = 0x0C,
    OFFSET_CAL_CIN1 = 0x0D,
    OFFSET_CAL_CIN2 = 0x0E,
    OFFSET_CAL_CIN3 = 0x0F,
    OFFSET_CAL_CIN4 = 0x10,
    GAIN_CAL_CIN1 = 0x11,
    GAIN_CAL_CIN2 = 0x12,
    GAIN_CAL_CIN3 = 0x13,
    GAIN_CAL_CIN4 = 0x14,
    MANUFACTURER_ID = 0xFE,
    DEVICE_ID = 0xFF
};

/** Template concept to constrain what structures encapsulate registers.*/
template <typename Reg>
// Struct has a valid register address
// Struct has an integer with the total number of bits in a register.
// This is used to mask the value before writing it to the sensor.
concept FDC1004CommandRegister =
    std::same_as<std::remove_cvref_t<decltype(Reg::address)>,
                 std::remove_cvref_t<Registers&>> &&
    std::integral<decltype(Reg::value_mask)> &&
    std::is_array_v<decltype(Reg::mode_map)>;

}  // namespace fdc1004

struct __attribute__((packed, __may_alias__)) ManufactureId {
    static constexpr Registers address = Registers::MANUFACTURER_ID;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t manufacture_id : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) DeviceId {
    static constexpr Registers address = Registers::MANUFACTURER_ID;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t device_id : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1_MSB {
    static constexpr Registers address = Registers::MEAS1_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1_LSB {
    static constexpr Registers address = Registers::MEAS1_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2_MSB {
    static constexpr Registers address = Registers::MEAS2_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2_LSB {
    static constexpr Registers address = Registers::MEAS2_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3_MSB {
    static constexpr Registers address = Registers::MEAS3_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3_LSB {
    static constexpr Registers address = Registers::MEAS3_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4_MSB {
    static constexpr Registers address = Registers::MEAS4_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4_LSB {
    static constexpr Registers address = Registers::MEAS4_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t measurement : 8 = 0;
    uint8_t padding : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure1 {
    static constexpr Registers address = Registers::CONF_MEAS1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure2 {
    static constexpr Registers address = Registers::CONF_MEAS2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure3 {
    static constexpr Registers address = Registers::CONF_MEAS3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure4 {
    static constexpr Registers address = Registers::CONF_MEAS4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Notes:

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // positive input channel
    uint8_t CHA : 3 = 0;
    // negative input channel
    uint8_t CHB : 3 = 0;
    uint8_t CAPDAC : 5 = 0;
    uint8_t padding_0 : 4 = 0;
};

struct __attribute__((packed, __may_alias__)) FDCConf {
    static constexpr Registers address = Registers::FDC_CONF;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t reset : 1 = 0;
	uint8_t padding_0 : 2 = 0;
	// Measurement rate Options:
	// b00 Reserved
    // b01 100S/s
    // b10 200S/s
    // b11 400S/s
	uint8_t measurement_rate : 2 = 0x1;
	uint8_t padding_1 : 1 = 0;
	uint8_t repeating_measurements: 1 = 0;
	uint8_t measure_mode_1: 1 = 0;
	uint8_t measure_mode_2: 1 = 0;
	uint8_t measure_mode_3: 1 = 0;
	uint8_t measure_mode_4: 1 = 0;
	uint8_t measure_mode_1_status: 1 = 0;
	uint8_t measure_mode_2_status: 1 = 0;
	uint8_t measure_mode_3_status: 1 = 0;
	uint8_t measure_mode_4_status: 1 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN1 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN2 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN3 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN4 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN1 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN2 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN3 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN4 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

	uint8_t offset_integer: 5 = 0;
	uint16_t offset_decimal: 11 = 0;
};

struct FDC1004RegisterMap {
    ManufactureId manufacture_id = {};
	DeviceId device_id = {};
    MeasureMode1_MSB measure_mode_1_msb = {};
    MeasureMode1_LSB measure_mode_1_lsb = {};
    MeasureMode2_MSB measure_mode_2_msb = {};
    MeasureMode2_LSB measure_mode_2_lsb = {};
    MeasureMode3_MSB measure_mode_3_msb = {};
    MeasureMode3_LSB measure_mode_3_lsb = {};
    MeasureMode4_MSB measure_mode_4_msb = {};
    MeasureMode4_LSB measure_mode_4_lsb = {};
    ConfMeasure1 config_measure_1 = {};
    ConfMeasure2 config_measure_2 = {};
    ConfMeasure3 config_measure_3 = {};
    ConfMeasure4 config_measure_4 = {};
    FDCConf fdc_conf = {};
    OffsetCalCIN1 offset_cal_cin1 = {};
    OffsetCalCIN2 offset_cal_cin2 = {};
    OffsetCalCIN3 offset_cal_cin3 = {};
    OffsetCalCIN4 offset_cal_cin4 = {};
	GainCalCIN1 gain_cal_cin1 = {};
    GainCalCIN2 gain_cal_cin2 = {};
    GainCalCIN3 gain_cal_cin3 = {};
    GainCalCIN4 gain_cal_cin4 = {};
    Reset reset = {};
};

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

constexpr uint8_t CAPDAC_MSB_MASK = 0x3;

constexpr uint8_t CAPDAC_LSB_MASK = 0xE0;

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
          ((sizeof(int32_t) * 8) - CONVERSION_BITS));

// Because we're doing big gantry moves, we'll probably have to handle
// the parasitic capacitance of the system shifting around a lot. We'll
// need to automatically reset our capdac to account for changing
// gross-scale capacitance conditions, and we'll do it by bumping up
// the capdac every time we get half way to either edge of our range.
constexpr float CAPDAC_REZERO_THRESHOLD_PF = MAX_MEASUREMENT_PF / 2;

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
    // Floor of 0
    offset_pf = std::max(offset_pf, 0.0F);
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
    return (DEVICE_CONFIGURATION_MSB | ((capdac_raw >> 3) & CAPDAC_MSB_MASK));
}

inline constexpr auto device_configuration_lsb(uint8_t capdac_raw) -> uint8_t {
    return (DEVICE_CONFIGURATION_LSB | ((capdac_raw << 5) & CAPDAC_LSB_MASK));
}

};  // namespace fdc1004
};  // namespace sensors
