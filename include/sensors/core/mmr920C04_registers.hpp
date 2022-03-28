/*
 * MMR920C04 Pressure Sensor
 *
 * Datasheet:
 * https://www.ti.com/lit/ds/symlink/fdc1004.pdf?ts=1645556558559&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FFDC1004
 *
 * The configuration of the sensor is always in single-ended measurement mode.
 * We are currently only using 1 out of the 4 measurement channels.
 *
 * By default, CAPDAC is enabled. The max resolution of the capdac is 5 bits.
 *
 * Pressure measurement is a total of 24 bits long.
 *
 * Filter coefficient equation:
 *
 * Fco = 2^27 * e^(-2pi & freq * tcondition)
 *
 * There are four pressure effective resolution modes which determines
 * tcondition in the filter coefficient equation.
 */
namespace mmr920C04_utils {

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
        MACRAM_WRITE = 0xE4};

    template <typename Reg>
    concept WritableRegister = requires() {
        {Reg::writable};
    };

    template <typename Reg>
    concept ReadableRegister = requires() {
        {Reg::readable};
    };

    // registers
    constexpr uint16_t ADDRESS = 0x67 << 1;

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

    struct __attribute__((packed, __may_alias__)) NormalPressureCommand {
        static constexpr Registers address = Registers::PRESSURE_READ;
        static constexpr bool readable = true;
        static constexpr bool writable = false;
        static constexpr uint32_t value_mask = (1 << 8) - 1;

        // Pascals per 1 cmH20
        constexpr CMH20_TO_PASCALS = 98.0665;
        static const float PA_PER_COUNT = 98.0665e-5;  // 1.0e-5cmH2O/count * 98.0665Pa/cmH2O

        uint8_t C7 : 1 = 1;
        uint8_t C6 : 1 = 1;
        uint8_t C5 : 1 = 0;
        uint8_t C4 : 1 = 0;
        uint8_t C3 : 1 = 0;
        uint8_t C2 : 1 = 1;
        uint8_t C1 : 1 = 0;
        uint8_t C0 : 1 = 0;

        [[nodiscard]] static auto to_pressure(uint32_t reg) -> float {
            // Sign extend pressure result
            if (measurement & 0x00800000)
                measurement |= 0xFF000000;
            else
                measurement &= 0x007FFFFF;

            float pressure = static_cast<float>(reg) * PA_PER_COUNT;
            return pressure; // convert to fixed point
        }
    };

    struct __attribute__((packed, __may_alias__)) NormalPressure {
        static constexpr Registers address = Registers::PRESSURE_READ;
        static constexpr bool readable = true;
        static constexpr bool writable = false;
        static constexpr uint32_t value_mask = (1 << 24) - 1;

        // Pascals per 1 cmH20
        constexpr CMH20_TO_PASCALS = 98.0665;
        static const float PA_PER_COUNT = 98.0665e-5;  // 1.0e-5cmH2O/count * 98.0665Pa/cmH2O

        uint32_t reading : 24 = 0;

        [[nodiscard]] static auto to_pressure(uint32_t reg) -> float {
            // Sign extend pressure result
            if (measurement & 0x00800000)
                measurement |= 0xFF000000;
            else
                measurement &= 0x007FFFFF;

            float pressure = static_cast<float>(reg) * PA_PER_COUNT;
            return pressure; // convert to fixed point
        }
    };

    struct __attribute__((packed, __may_alias__)) LowPassPressureCommand {
        static constexpr Registers address = Registers::LOW_PASS_PRESSURE_READ;
        static constexpr bool readable = true;
        static constexpr bool writable = false;
        static constexpr uint32_t value_mask = (1 << 8) - 1;

        uint8_t C7 : 1 = 1;
        uint8_t C6 : 1 = 1;
        uint8_t C5 : 1 = 0;
        uint8_t C4 : 1 = 0;
        uint8_t C3 : 1 = 0;
        uint32_t C2 : 1 = 1;
        uint32_t C1 : 1 = 0;
        uint32_t C0 : 1 = 0;
    };

    struct __attribute__((packed, __may_alias__)) LowPassPressure {
        static constexpr Registers address = Registers::LOW_PASS_PRESSURE_READ;
        static constexpr bool readable = true;
        static constexpr bool writable = false;
        static constexpr uint32_t value_mask = (1 << 24) - 1;

        // Pascals per 1 cmH20
        constexpr CMH20_TO_PASCALS = 98.0665;
        static const float PA_PER_COUNT = 98.0665e-5;  // 1.0e-5cmH2O/count * 98.0665Pa/cmH2O

        uint32_t reading : 24 = 0;

        [[nodiscard]] static auto to_pressure(uint32_t reg) -> float {
            // Sign extend pressure result
            if (measurement & 0x00800000)
                measurement |= 0xFF000000;
            else
                measurement &= 0x007FFFFF;

            float pressure = static_cast<float>(reg) * PA_PER_COUNT;
            return pressure; // convert to fixed point
        }
    };

    struct __attribute__((packed, __may_alias__)) TemperatureCommand {
        static constexpr Registers address = Registers::TEMPERATURE_READ;
        static constexpr bool readable = true;
        static constexpr bool writable = false;
        static constexpr uint32_t value_mask = (1 << 24) - 1;

        uint8_t C7 : 1 = 1;
        uint8_t C6 : 1 = 1;
        uint8_t C5 : 1 = 0;
        uint8_t C4 : 1 = 0;
        uint8_t C3 : 1 = 0;
        uint32_t C2 : 1 = 1;
        uint32_t C1 : 1 = 0;
        uint32_t C0 : 1 = 0;

        [[nodiscard]] static auto to_temperature(uint32_t reg) -> float {
            // Sign extend pressure result
            if (measurement & 0x00800000)
                measurement |= 0xFF000000;
            else
                measurement &= 0x007FFFFF;

            float pressure = static_cast<float>(reg) * PA_PER_COUNT;
            return pressure; // convert to fixed point
        }
    };

    struct __attribute__((packed, __may_alias__)) Temperature {
        static constexpr Registers address = Registers::TEMPERATURE_READ;
        static constexpr bool readable = true;
        static constexpr bool writable = false;
        static constexpr uint32_t value_mask = (1 << 24) - 1;

        static constexpr uint32_t MAX_SIZE = (2 << 7);

        uint32_t reading: 24 = 0;

        [[nodiscard]] static auto to_temperature(uint32_t reg) -> float {
            // Sign extend pressure result
            if (measurement & 0x00800000)
                measurement |= 0xFF000000;
            else
                measurement &= 0x007FFFFF;

            float pressure = static_cast<float>(reg) / MAX_SIZE;
            return pressure; // convert to fixed point
        }
    };

    struct MMR920C04RegisterMap {
        Reset reset = {};
        Idle idle = {};
        MeasureMode1 measure_mode_1 = {};
        MeasureMode2 measure_mode_2 = {};
        MeasureMode3 measure_mode_3 = {};
        MeasureMode4 measure_mode_4 = {};
        NormalPressureCommand pressure_command = {};
        NormalPressure pressure = {};
        LowPassPressureCommand low_pass_pressure_command = {};
        LowPassPressure low_pass_pressure = {};
        TemperatureCommand temperature_command = {};
        Temperature temperature = {};
    };

} // namespace mmr920C04_registers