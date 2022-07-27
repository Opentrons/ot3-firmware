#pragma once
/*
 * HDC3020 Relative Humidity & Temperature Sensor
 *
 * Datasheet:
 * https://www.ti.com/lit/ds/symlink/hdc3020.pdf?ts=1658853555423
 *
 * Temperature and Humidity measurement are 16 bits long respectively.
 *
 * RH Equation:
 *
 * RH(%) = 100 x [input * 1/(2^16 - 1)]
 *
 * Temperature (ºC) Equation:
 *
 * -45 + [175 * (input/2^16 - 1)]
 *
 * Each command, whether trigger mode or continuous mode must
 * send an MSB register and LSB register. The LSB register always represents
 * the power mode to use for measurement.
 */
namespace sensors {
namespace hdc3020 {
constexpr uint16_t ADDRESS = 0x44 << 1;

enum class LowPowerMode : uint8_t {
    /*
     * Power consumption
     * Mode 0 -> highest; Mode 3 -> lowest
     *
     * Noise
     * Mode 0 -> lowest; Mode 3 -> highest
     */
    ZERO,
    ONE,
    TWO,
    THREE
};

enum class Registers : uint8_t {
    // 0xE0 or 0x0E
    AUTO_MEASURE_STATUS = 0xE0,
    TRIGGER_ON_DEMAND_MODE = 0x24,
    // 1 measurement per 2 sec
    AUTO_MEASURE_1M2S = 0x20,
    // 1 measurement per sec
    AUTO_MEASURE_1M1S = 0x21,
    // 2 measurement per sec
    AUTO_MEASURE_2M1S = 0x22,
    // 4 measurement per sec
    AUTO_MEASURE_4M1S = 0x23,
    // 10 measurement per sec
    AUTO_MEASURE_10M1S = 0x27,
    CONFIGURE_THRESHOLD = 0x61,
    RESET = 0x30,
    READ_STATUS_REGISTER = 0xF3,
    CLEAR_STATUS_REGISTER = 0x30,
    OFFSET_PROGRAM = 0xA0,
    MANUFACTURE_ID = 0x37,
    NIST_ID = 0x36

};

/** Template concept to constrain what structures encapsulate registers.*/
template <typename Reg>
// Struct has a valid register address
// Struct has an integer with the total number of bits in a register.
// This is used to mask the value before writing it to the sensor.
concept HDC3020CommandRegister =
    std::same_as<std::remove_cvref_t<decltype(Reg::address)>,
                 std::remove_cvref_t<Registers&>> &&
    std::integral<decltype(Reg::value_mask)> &&
    std::is_array_v<decltype(Reg::mode_map)>;

struct __attribute__((packed, __may_alias__)) ManufactureId {
    // is this a status??
    static constexpr Registers address = Registers::MANUFACTURE_ID;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) AutoMeasureStatus {
    // is this a status??
    static constexpr Registers address = Registers::AUTO_MEASURE_STATUS;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t initialize : 1 = 1;
    uint8_t minimum_temperature : 1 = 0;
    uint8_t maximum_temperature : 1 = 0;
    uint8_t minimum_rh : 1 = 1;
    uint8_t maximum_rh : 1 = 1;
};

struct __attribute__((packed, __may_alias__)) TriggerOnDemandMeasure {
    // is this a status??
    static constexpr Registers address = Registers::AUTO_MEASURE_STATUS;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;
    // M0, M1, M2, M3 power modes
    static constexpr uint8_t mode_map[4] = {0x00, 0x0B, 0x16, 0xFF};

    uint16_t temperature : 16 = 0;
    uint16_t humidity : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) AutoMeasure1M1S {
    static constexpr Registers address = Registers::AUTO_MEASURE_1M1S;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;
    // M0, M1, M2, M3 power modes
    static constexpr uint8_t mode_map[4] = {0x30, 0x26, 0x2D, 0xFF};

    uint16_t temperature : 16 = 0;
    uint16_t humidity : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) AutoMeasure1M2S {
    static constexpr Registers address = Registers::AUTO_MEASURE_1M2S;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;
    // M0, M1, M2, M3 power modes
    static constexpr uint8_t mode_map[4] = {0x32, 0x24, 0x2F, 0xFF};

    uint16_t temperature : 16 = 0;
    uint16_t humidity : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) AutoMeasure2M1S {
    static constexpr Registers address = Registers::AUTO_MEASURE_2M1S;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;
    // M0, M1, M2, M3 power modes
    static constexpr uint8_t mode_map[4] = {0x36, 0x20, 0x2B, 0xFF};

    uint16_t temperature : 16 = 0;
    uint16_t humidity : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) AutoMeasure4M1S {
    static constexpr Registers address = Registers::AUTO_MEASURE_4M1S;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;
    // M0, M1, M2, M3 power modes
    static constexpr uint8_t mode_map[4] = {0x34, 0x22, 0x29, 0xFF};

    uint16_t temperature : 16 = 0;
    uint16_t humidity : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) AutoMeasure10M1S {
    static constexpr Registers address = Registers::AUTO_MEASURE_10M1S;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;
    // M0, M1, M2, M3 power modes
    static constexpr uint8_t mode_map[4] = {0x37, 0x21, 0x2A, 0xFF};

    uint16_t temperature : 16 = 0;
    uint16_t humidity : 16 = 0;
};

struct __attribute__((packed, __may_alias__)) Reset {
    static constexpr Registers address = Registers::RESET;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t reset : 8 = 0x93;
};

struct HDC3020RegisterMap {
    ManufactureId manufacture_id = {};
    AutoMeasureStatus status = {};
    TriggerOnDemandMeasure trigger_measurement = {};
    AutoMeasure1M1S measure_mode_1m1s = {};
    AutoMeasure1M2S measure_mode_1m2s = {};
    AutoMeasure2M1S measure_mode_2m1s = {};
    AutoMeasure4M1S measure_mode_4m1s = {};
    AutoMeasure10M1S measure_mode_10m1s = {};
    Reset reset = {};
};

// Registers are all 32 bits
using RegisterSerializedType = uint32_t;
// Type definition to allow type aliasing for pointer dereferencing
using RegisterSerializedTypeA = __attribute__((__may_alias__)) uint32_t;
};  // namespace hdc3020
};  // namespace sensors
