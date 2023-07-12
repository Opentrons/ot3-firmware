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

constexpr uint16_t ADDRESS = 0x50 << 1;

// Max reading including the offset for the sensor is approximately
// 115 pF.
constexpr float MAX_CAPACITANCE_READING = 115.0F;

enum class CHA : uint8_t {
    CIN1 = 0x0,
    CIN2 = 0x1,
    CIN3 = 0x2,
    CIN4 = 0x3,
};

enum class CHB : uint8_t {
    CIN1 = 0x0,
    CIN2 = 0x1,
    CIN3 = 0x2,
    CIN4 = 0x3,
    CAPDAC = 0x4,
    DISABLED = 0x8
};

enum class MeasurementRate : uint8_t {
    // b00 Reserved
    // b01 100S/s
    // b10 200S/s
    // b11 400S/s
    RESERVED = 0x0,
    ONE_HUNDRED_SAMPLES_PER_SECOND = 0x1,
    TWO_HUNDRED_SAMPLES_PER_SECOND = 0x2,
    FOUR_HUNDRED_SAMPLES_PER_SECOND = 0x3,
};

// This is used to determine which configuration register
// you're currently using and thus what measurement mode
// you're in.
enum class MeasureConfigMode : uint8_t { ONE, TWO, THREE, FOUR };

// Capacitance Sensor Address and Registers
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

static inline auto is_valid_address(const uint8_t add) -> bool {  // NOLINT
    switch (static_cast<Registers>(add)) {
        case Registers::MANUFACTURER_ID:
        case Registers::DEVICE_ID:
        case Registers::MEAS1_MSB:
        case Registers::MEAS1_LSB:
        case Registers::MEAS2_MSB:
        case Registers::MEAS2_LSB:
        case Registers::MEAS3_MSB:
        case Registers::MEAS3_LSB:
        case Registers::MEAS4_MSB:
        case Registers::MEAS4_LSB:
        case Registers::CONF_MEAS1:
        case Registers::CONF_MEAS2:
        case Registers::CONF_MEAS3:
        case Registers::CONF_MEAS4:
        case Registers::FDC_CONF:
        case Registers::OFFSET_CAL_CIN1:
        case Registers::OFFSET_CAL_CIN2:
        case Registers::OFFSET_CAL_CIN3:
        case Registers::OFFSET_CAL_CIN4:
        case Registers::GAIN_CAL_CIN1:
        case Registers::GAIN_CAL_CIN2:
        case Registers::GAIN_CAL_CIN3:
        case Registers::GAIN_CAL_CIN4:
            return true;
    }
    return false;
}

/** Template concept to constrain what structures encapsulate registers.*/
template <typename Reg>
// Struct has a valid register address
// Struct has an integer with the total number of bits in a register.
// This is used to mask the value before writing it to the sensor.
concept FDC1004Register =
    std::same_as<std::remove_cvref_t<decltype(Reg::address)>,
                 std::remove_cvref_t<Registers &>> &&
    std::integral<decltype(Reg::value_mask)>;

template <typename Reg>
concept WritableRegister = requires() {
    {Reg::writable};
};

template <typename Reg>
concept ReadableRegister = requires() {
    {Reg::readable};
};

struct __attribute__((packed, __may_alias__)) ManufactureId {
    static constexpr Registers address = Registers::MANUFACTURER_ID;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint16_t manufacture_id : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) DeviceId {
    static constexpr Registers address = Registers::DEVICE_ID;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint16_t device_id : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1_MSB {
    static constexpr Registers address = Registers::MEAS1_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1_LSB {
    static constexpr Registers address = Registers::MEAS1_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint8_t padding : 8 = 0;
    uint8_t measurement : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2_MSB {
    static constexpr Registers address = Registers::MEAS2_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2_LSB {
    static constexpr Registers address = Registers::MEAS2_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint8_t padding : 8 = 0;
    uint8_t measurement : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3_MSB {
    static constexpr Registers address = Registers::MEAS3_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3_LSB {
    static constexpr Registers address = Registers::MEAS3_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint8_t padding : 8 = 0;
    uint8_t measurement : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4_MSB {
    static constexpr Registers address = Registers::MEAS4_MSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint16_t measurement : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4_LSB {
    static constexpr Registers address = Registers::MEAS4_LSB;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint8_t padding : 8 = 0;
    uint8_t measurement : 8 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure1 {
    static constexpr Registers address = Registers::CONF_MEAS1;
    static constexpr bool readable = true;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // Notes:

    // You can store up to 4 measurement configurations on the cap sensor
    // using the 4 different conf measurement registers.

    // (1) It is not permitted to configure a measurement where the CHA field
    // and CHB field hold the same value (for example, if CHA=b010, CHB cannot
    // also be set to b010). (2) It is not permitted to configure a differential
    // measurement between CHA and CHB where CHA > CHB (for example, if CHA=
    // b010, CHB cannot be b001 or b000).

    // Single ended measurement capacitive offset is CAPDAC x 3.125 pF
    // The max offset is 96.875 pF

    uint16_t padding_0 : 5 = 0;
    uint16_t CAPDAC : 5 = 0;
    // negative input channel
    uint16_t CHB : 3 = 0;
    // positive input channel
    uint16_t CHA : 3 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure2 {
    static constexpr Registers address = Registers::CONF_MEAS2;
    static constexpr bool readable = true;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See ConfMeasure1

    uint16_t padding_0 : 5 = 0;
    uint16_t CAPDAC : 5 = 0;
    // negative input channel
    uint16_t CHB : 3 = 0;
    // positive input channel
    uint16_t CHA : 3 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure3 {
    static constexpr Registers address = Registers::CONF_MEAS3;
    static constexpr bool readable = true;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See ConfMeasure1

    uint16_t padding_0 : 5 = 0;
    uint16_t CAPDAC : 5 = 0;
    // negative input channel
    uint16_t CHB : 3 = 0;
    // positive input channel
    uint16_t CHA : 3 = 0;
};

struct __attribute__((packed, __may_alias__)) ConfMeasure4 {
    static constexpr Registers address = Registers::CONF_MEAS4;
    static constexpr bool readable = true;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See ConfMeasure1

    uint16_t padding_0 : 5 = 0;
    uint16_t CAPDAC : 5 = 0;
    // negative input channel
    uint16_t CHB : 3 = 0;
    // positive input channel
    uint16_t CHA : 3 = 0;
};

struct __attribute__((packed, __may_alias__)) FDCConf {
    static constexpr Registers address = Registers::FDC_CONF;
    static constexpr bool readable = true;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    uint16_t measure_mode_4_status : 1 = 0;
    uint16_t measure_mode_3_status : 1 = 0;
    uint16_t measure_mode_2_status : 1 = 0;
    uint16_t measure_mode_1_status : 1 = 0;

    // The measurement mode you wish to run in
    uint16_t measure_mode_4 : 1 = 0;
    uint16_t measure_mode_3 : 1 = 0;
    uint16_t measure_mode_2 : 1 = 0;
    uint16_t measure_mode_1 : 1 = 0;

    uint16_t repeating_measurements : 1 = 0;
    uint16_t padding_1 : 1 = 0;

    // Measurement rate Options:

    // See values in `MeasurementRate`

    uint16_t measurement_rate : 2 = 0;

    uint16_t padding_0 : 3 = 0;
    uint16_t reset : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN1 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // -16pF -> 16pF range. Used to reduce parasitic capacitance due to
    // external circuitry

    // The value in the register is stored as a fixed point integer
    // in 5Q11 format.
    uint16_t offset_decimal : 11 = 0;
    uint16_t offset_integer : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN2 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See OffsetCalCIN1
    uint16_t offset_decimal : 11 = 0;
    uint16_t offset_integer : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN3 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See OffsetCalCIN1
    uint16_t offset_decimal : 11 = 0;
    uint16_t offset_integer : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) OffsetCalCIN4 {
    static constexpr Registers address = Registers::OFFSET_CAL_CIN4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See OffsetCalCIN1
    uint16_t offset_decimal : 11 = 0;
    uint16_t offset_integer : 5 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN1 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // Gain factor correction ranging from 0 to 4 that can be applied
    // to individual channels to remove gain mismatch.

    // The register is stored as a fixed point value in 2Q14 format.
    // The gain is calculated by Gain = GAIN[15:0]/2^14
    uint16_t offset_decimal : 14 = 0;
    uint16_t offset_integer : 2 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN2 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See GainCalCIN1
    uint16_t offset_decimal : 14 = 0;
    uint16_t offset_integer : 2 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN3 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See GainCalCIN1
    uint16_t offset_decimal : 14 = 0;
    uint16_t offset_integer : 2 = 0;
};

struct __attribute__((packed, __may_alias__)) GainCalCIN4 {
    static constexpr Registers address = Registers::GAIN_CAL_CIN4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint16_t value_mask = 0xFFFF;

    // See GainCalCIN1
    uint16_t offset_decimal : 14 = 0;
    uint16_t offset_integer : 2 = 0;
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
};

// Registers are all 16 bits
using RegisterSerializedType = uint16_t;
// Type definition to allow type aliasing for pointer dereferencing
using RegisterSerializedTypeA = __attribute__((__may_alias__)) uint16_t;

};  // namespace fdc1004

namespace fdc1004_utils {
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

inline auto measurement_ready(fdc1004::FDCConf &fdc,
                              fdc1004::MeasureConfigMode mode) -> bool {
    switch (mode) {
        case fdc1004::MeasureConfigMode::ONE:
            return fdc.measure_mode_1_status;
        case fdc1004::MeasureConfigMode::TWO:
            return fdc.measure_mode_2_status;
        case fdc1004::MeasureConfigMode::THREE:
            return fdc.measure_mode_3_status;
        case fdc1004::MeasureConfigMode::FOUR:
            return fdc.measure_mode_4_status;
    }
    return false;
}

}  // namespace fdc1004_utils
};  // namespace sensors
