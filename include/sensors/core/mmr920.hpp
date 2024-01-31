#pragma once
// TODO (lc 02-16-2022) We should refactor the fixed point
// helper functions such that they live in a shared location.

#include "motor-control/core/utils.hpp"
/*
 * MMR920 Pressure Sensor
 *
 * Datasheet:
 * https://nmbtc.com/wp-content/uploads/2021/03/mmr920_leaflet_e_rev1.pdf
 * (Search in our google drive for a more comprehensive datasheet).
 *
 * Pressure measurement is a total of 24 bits long.
 *
 * Filter coefficient equation:
 *
 * Fco = 2^27 * e^(-2pi & freq * tcondition)
 *
 * There are four pressure effective resolution modes which determines
 * tcondition in the filter coefficient equation.
 *
 * The register structs below are pre-built with the command message
 * required to get the information from the sensor.
 *
 * The command bits (C7->C0) are another way of stating the 'register' address.
 * Leaving them alone for now.
 */
namespace sensors {
namespace mmr920 {
constexpr uint16_t ADDRESS = 0x67 << 1;

// Pressure cannot be measured beyond +/-8226.4F on the old sensors
// New sensors have double the threshold but half the fidelity

enum class SensorVersion : int {
    mmr920c04 = 0,
    mmr920c10 = 1,
};

[[nodiscard]] inline static auto get_max_pressure_reading(SensorVersion version)
    -> float {
    if (version == SensorVersion::mmr920c10) {
        return 16452.8F;
    } else {
        return 8226.4F;
    }
}

enum class SensorStatus : uint8_t {
    SHUTDOWN = 0x0,
    IDLE = 0xE5,
    ACTIVE = 0xED,
    UNKNOWN = 0xFF
};

enum class FilterSetting : uint8_t {
    NO_FILTER,
    /// @brief Cutoff Frequency
    // The cutoff frequency filter coefficient varies
    // depending on the measure mode you are using.
    // mode, no filter, fc@10Hz, fc@100Hz
    // MODE1, 0.019, 0.012, 0.0068
    // MODE2, 0.008, 0.0064, 0.0034
    // MODE3, 0.0044, 0.0036, 0.0022
    // MODE4, 0.0025, 0.0023, 0.0013
    LOW_PASS_FILTER
};

enum class MeasurementRate : int {
    MEASURE_1 = 0,  // 0.405msec
    MEASURE_2 = 1,  // 0.810msec
    MEASURE_3 = 2,  // 1.62msec
    MEASURE_4 = 3   // 3.24msec
};

enum class Registers : uint8_t {
    RESET = 0x72,
    IDLE = 0x94,
    MEASURE_MODE_1 = 0xA0,
    MEASURE_MODE_2 = 0xA2,
    MEASURE_MODE_3 = 0xA4,
    MEASURE_MODE_4 = 0xA6,
    PRESSURE_READ = 0xC0,
    LOW_PASS_PRESSURE_READ = 0xC4,
    TEMPERATURE_READ = 0xC2,
    STATUS = 0x80,
    MACRAM_WRITE = 0xE4
};

static inline auto is_valid_address(const uint8_t add) -> bool {  // NOLINT
    switch (static_cast<Registers>(add)) {
        case Registers::RESET:
        case Registers::IDLE:
        case Registers::MEASURE_MODE_1:
        case Registers::MEASURE_MODE_2:
        case Registers::MEASURE_MODE_3:
        case Registers::MEASURE_MODE_4:
        case Registers::PRESSURE_READ:
        case Registers::LOW_PASS_PRESSURE_READ:
        case Registers::TEMPERATURE_READ:
        case Registers::STATUS:
        case Registers::MACRAM_WRITE:
            return true;
    }
    return false;
}

/** Template concept to constrain what structures encapsulate registers.*/
template <typename Reg>
// Struct has a valid register address
// Struct has an integer with the total number of bits in a register.
// This is used to mask the value before writing it to the sensor.
concept MMR920CommandRegister =
    std::same_as<std::remove_cvref_t<decltype(Reg::address)>,
                 std::remove_cvref_t<Registers&>> &&
    std::integral<decltype(Reg::value_mask)>;

struct __attribute__((packed, __may_alias__)) Reset {
    static constexpr Registers address = Registers::RESET;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
    uint8_t C6 : 1 = 1;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 1;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 0;
    uint8_t C1 : 1 = 1;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) Idle {
    static constexpr Registers address = Registers::IDLE;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 0;
    uint8_t C4 : 1 = 1;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 1;
    uint8_t C1 : 1 = 0;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1 {
    static constexpr Registers address = Registers::MEASURE_MODE_1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 0;
    uint8_t C1 : 1 = 0;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2 {
    static constexpr Registers address = Registers::MEASURE_MODE_2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 0;
    uint8_t C1 : 1 = 1;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3 {
    static constexpr Registers address = Registers::MEASURE_MODE_3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 1;
    uint8_t C1 : 1 = 0;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4 {
    static constexpr Registers address = Registers::MEASURE_MODE_4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 1;
    uint8_t C1 : 1 = 1;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) PressureCommand {
    static constexpr Registers address = Registers::PRESSURE_READ;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 1;
    uint8_t C5 : 1 = 0;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 0;
    uint8_t C1 : 1 = 0;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) LowPassPressureCommand {
    static constexpr Registers address = Registers::LOW_PASS_PRESSURE_READ;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 1;
    uint8_t C5 : 1 = 0;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 0;
    uint8_t C1 : 1 = 0;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) TemperatureCommand {
    static constexpr Registers address = Registers::TEMPERATURE_READ;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 1;
    uint8_t C5 : 1 = 0;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 0;
    uint8_t C1 : 1 = 1;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) StatusCommand {
    static constexpr Registers address = Registers::STATUS;
    static constexpr bool writable = true;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 0;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 0;
    uint8_t C1 : 1 = 0;
    uint8_t C0 : 1 = 0;
};

struct PressureResult {
    // Pascals per 1 cmH20
    static constexpr float CMH20_TO_PASCALS = 98.0665;
    uint32_t reading : 32 = 0;

    [[nodiscard]] static auto get_pa_per_count(SensorVersion version) -> float {
        // conversion factor of a given 3 byte measurement to Pascals
        if (version == SensorVersion::mmr920c10) {
            return 2 * 1e-5 *
                   CMH20_TO_PASCALS;  // 1.0e-5cmH2O/count * 98.0665Pa/cmH2O
        }
        return 1e-5 * CMH20_TO_PASCALS;  // 1.0e-5cmH2O/count * 98.0665Pa/cmH2O
    }

    [[nodiscard]] static auto to_pressure(uint32_t reg, SensorVersion version)
        -> float {
        // Pressure is converted to pascals
        // Sign extend pressure result
        if ((reg & 0x00800000) != 0) {
            reg |= 0xFF000000;
        } else {
            reg &= 0x007FFFFF;
        }

        float pressure = static_cast<float>(static_cast<int32_t>(reg)) *
                         get_pa_per_count(version);
        return pressure;
    }
};

struct TemperatureResult {
    static constexpr uint32_t MAX_SIZE = (2 << 7);
    static constexpr float CONVERT_TO_CELSIUS = 0.0078125;

    uint32_t reading : 32 = 0;

    [[nodiscard]] static auto to_temperature(uint32_t reg) -> float {
        // Pressure is converted to pascals
        // Sign extend pressure result
        if ((reg & 0x00800000) != 0) {
            reg |= 0xFF000000;
        } else {
            reg &= 0x007FFFFF;
        }
        float temperature =
            CONVERT_TO_CELSIUS * (static_cast<float>(reg) / MAX_SIZE);
        return temperature;
    }
};

struct StatusResult {
    uint8_t reading : 8 = 0;

    [[nodiscard]] static auto to_status(uint8_t reg) -> SensorStatus {
        switch (static_cast<SensorStatus>(reg)) {
            case SensorStatus::IDLE:
                return SensorStatus::IDLE;
            case SensorStatus::ACTIVE:
                return SensorStatus::ACTIVE;
            case SensorStatus::SHUTDOWN:
                return SensorStatus::SHUTDOWN;
            default:
                return SensorStatus::UNKNOWN;
        }
    }
};

[[nodiscard]] inline static auto reading_to_fixed_point(float reading)
    -> sq15_16 {
    return convert_to_fixed_point(reading, S15Q16_RADIX);
}

struct MMR920RegisterMap {
    Reset reset = {};
    Idle idle = {};
    MeasureMode1 measure_mode_1 = {};
    MeasureMode2 measure_mode_2 = {};
    MeasureMode3 measure_mode_3 = {};
    MeasureMode4 measure_mode_4 = {};
    PressureCommand pressure_command = {};
    LowPassPressureCommand low_pass_pressure_command = {};
    TemperatureCommand temperature_command = {};
    StatusCommand status_command = {};
    PressureResult pressure_result = {};
    TemperatureResult temperature_result = {};
    StatusResult status_result = {};
};

// Result registers are a mixture of
using RegisterSerializedType = uint8_t;

// Command Registers are all 8 bits
// Type definition to allow type aliasing for pointer dereferencing
using RegisterSerializedTypeA = __attribute__((__may_alias__)) uint8_t;
};  // namespace mmr920
};  // namespace sensors
