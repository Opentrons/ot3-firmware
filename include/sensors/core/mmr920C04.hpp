#pragma once
/*
 * MMR920C04 Pressure Sensor
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
namespace mmr920C04 {
constexpr uint16_t ADDRESS = 0x67 << 1;

enum class SensorStatus : uint8_t {
    SHUTDOWN = 0x0,
    IDLE = 0xE5,
    ACTIVE = 0xED,
    UNKNOWN = 0xFF
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

static auto is_valid_address(const uint8_t add) -> bool {
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
concept MMR920C04Register = requires(Reg& r, uint32_t value) {
    // Struct has a valid register address
    std::same_as<decltype(Reg::address), Registers&>;
    // Struct has an integer with the total number of bits in a register.
    // This is used to mask the value before writing it to the sensor.
    std::integral<decltype(Reg::value_mask)>;
};

struct __attribute__((packed, __may_alias__)) Reset {
    static constexpr Registers address = Registers::RESET;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 0;
    uint8_t C6 : 1 = 1;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 1;
    uint8_t C3 : 1 = 0;
    uint32_t C2 : 1 = 0;
    uint32_t C1 : 1 = 1;
    uint32_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) Idle {
    static constexpr Registers address = Registers::IDLE;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 0;
    uint8_t C4 : 1 = 1;
    uint8_t C3 : 1 = 0;
    uint32_t C2 : 1 = 1;
    uint32_t C1 : 1 = 0;
    uint32_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode1 {
    static constexpr Registers address = Registers::MEASURE_MODE_1;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint32_t C2 : 1 = 0;
    uint32_t C1 : 1 = 0;
    uint32_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode2 {
    static constexpr Registers address = Registers::MEASURE_MODE_2;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint32_t C2 : 1 = 0;
    uint32_t C1 : 1 = 1;
    uint32_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode3 {
    static constexpr Registers address = Registers::MEASURE_MODE_3;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint32_t C2 : 1 = 1;
    uint32_t C1 : 1 = 0;
    uint32_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) MeasureMode4 {
    static constexpr Registers address = Registers::MEASURE_MODE_4;
    static constexpr bool readable = false;
    static constexpr bool writable = true;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    uint8_t C7 : 1 = 1;
    uint8_t C6 : 1 = 0;
    uint8_t C5 : 1 = 1;
    uint8_t C4 : 1 = 0;
    uint8_t C3 : 1 = 0;
    uint8_t C2 : 1 = 1;
    uint8_t C1 : 1 = 1;
    uint8_t C0 : 1 = 0;
};

struct __attribute__((packed, __may_alias__)) Pressure {
    static constexpr Registers address = Registers::PRESSURE_READ;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Pascals per 1 cmH20
    static constexpr float CMH20_TO_PASCALS = 98.0665;
    static constexpr float PA_PER_COUNT =
        1e-5 * CMH20_TO_PASCALS;  // 1.0e-5cmH2O/count * 98.0665Pa/cmH2O

    uint32_t C7 : 1 = 1;
    uint32_t C6 : 1 = 1;
    uint32_t C5 : 1 = 0;
    uint32_t C4 : 1 = 0;
    uint32_t C3 : 1 = 0;
    uint32_t C2 : 1 = 0;
    uint32_t C1 : 1 = 0;
    uint32_t C0 : 1 = 0;
    uint32_t reading : 24 = 0;

    [[nodiscard]] static auto to_pressure(uint32_t reg) -> sq14_15 {
        // Sign extend pressure result
        if (reg & 0x00800000)
            reg |= 0xFF000000;
        else
            reg &= 0x007FFFFF;

        float pressure = static_cast<float>(reg) * PA_PER_COUNT;
        return convert_to_fixed_point(pressure, 15);
    }
};

struct __attribute__((packed, __may_alias__)) LowPassPressure {
    static constexpr Registers address = Registers::LOW_PASS_PRESSURE_READ;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 8) - 1;

    // Pascals per 1 cmH20
    static constexpr float CMH20_TO_PASCALS = 98.0665;
    static constexpr float PA_PER_COUNT =
        1e-5 * CMH20_TO_PASCALS;  // 1.0e-5cmH2O/count * 98.0665Pa/cmH2O

    uint32_t C7 : 1 = 1;
    uint32_t C6 : 1 = 1;
    uint32_t C5 : 1 = 0;
    uint32_t C4 : 1 = 0;
    uint32_t C3 : 1 = 0;
    uint32_t C2 : 1 = 1;
    uint32_t C1 : 1 = 0;
    uint32_t C0 : 1 = 0;
    uint32_t reading : 24 = 0;

    [[nodiscard]] static auto to_pressure(uint32_t reg) -> sq14_15 {
        // Sign extend pressure result
        if (reg & 0x00800000)
            reg |= 0xFF000000;
        else
            reg &= 0x007FFFFF;

        float pressure = static_cast<float>(reg) * PA_PER_COUNT;
        return convert_to_fixed_point(pressure, 15);
    }
};

struct __attribute__((packed, __may_alias__)) Temperature {
    static constexpr Registers address = Registers::TEMPERATURE_READ;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint32_t value_mask = (1 << 24) - 1;

    static constexpr uint32_t MAX_SIZE = (2 << 7);

    static constexpr float CONVERT_TO_CELSIUS = 0.0078125;

    uint32_t C7 : 1 = 1;
    uint32_t C6 : 1 = 1;
    uint32_t C5 : 1 = 0;
    uint32_t C4 : 1 = 0;
    uint32_t C3 : 1 = 0;
    uint32_t C2 : 1 = 0;
    uint32_t C1 : 1 = 1;
    uint32_t C0 : 1 = 0;
    uint32_t reading : 24 = 0;

    [[nodiscard]] static auto to_temperature(uint32_t reg) -> sq14_15 {
        float temperature =
            CONVERT_TO_CELSIUS * (static_cast<float>(reg) / MAX_SIZE);
        return convert_to_fixed_point(temperature, 15);
    }
};

struct __attribute__((packed, __may_alias__)) Status {
    static constexpr Registers address = Registers::STATUS;
    static constexpr bool readable = true;
    static constexpr bool writable = false;
    static constexpr uint8_t value_mask = (1 << 8) - 1;

    uint32_t C7 : 1 = 1;
    uint32_t C6 : 1 = 0;
    uint32_t C5 : 1 = 0;
    uint32_t C4 : 1 = 0;
    uint32_t C3 : 1 = 0;
    uint32_t C2 : 1 = 0;
    uint32_t C1 : 1 = 0;
    uint32_t C0 : 1 = 0;
    uint32_t reading : 8 = 0;

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

struct MMR920C04RegisterMap {
    Reset reset = {};
    Idle idle = {};
    MeasureMode1 measure_mode_1 = {};
    MeasureMode2 measure_mode_2 = {};
    MeasureMode3 measure_mode_3 = {};
    MeasureMode4 measure_mode_4 = {};
    Pressure pressure = {};
    LowPassPressure low_pass_pressure = {};
    Temperature temperature = {};
    Status status = {};
};

// Registers are all 32 bits
using RegisterSerializedType = uint32_t;
// Type definition to allow type aliasing for pointer dereferencing
using RegisterSerializedTypeA = __attribute__((__may_alias__)) uint32_t;

}  // namespace mmr920C04