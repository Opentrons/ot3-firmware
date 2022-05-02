#pragma once

#include <cstdint>

namespace can {
namespace bit_timings {

// helper functions and values for calculating bit timings
static constexpr uint32_t NS_PER_S = 1000 * 1000 * 1000;

template <uint32_t can_clock_hz, uint32_t time_quantum_ns>
consteval auto get_clock_divider() -> uint8_t {
    static_assert(can_clock_hz != 0, "clock must not be 0");
    static_assert(time_quantum_ns != 0, "quantum time must not be 0");
    constexpr auto clockdivider =
        static_cast<uint32_t>((static_cast<uint64_t>(can_clock_hz) *
                               static_cast<uint64_t>(time_quantum_ns)) /
                              NS_PER_S);
    static_assert(clockdivider != 0,
                  "Cannot accomplish this quantum at this clock (too slow)");
    static_assert(clockdivider < 256,
                  "Cannot accomplish this quantum at this clock (too fast)");
    return clockdivider;
}

template <uint32_t can_clock_hz, uint32_t clockdiv, uint32_t time_quantum_ns>
consteval auto get_actual_time_quantum() -> uint32_t {
    static_assert(can_clock_hz != 0);
    constexpr auto actual_time_quantum = NS_PER_S / (can_clock_hz / clockdiv);
    // fails if the predicate is false (time quantum is outside bounds)
    static_assert(
        actual_time_quantum < (time_quantum_ns + (time_quantum_ns >> 3)),
        "Time quantum >12% too high");
    static_assert(
        actual_time_quantum > (time_quantum_ns - (time_quantum_ns >> 3)),
        "Time quantum >12% too low");
    return actual_time_quantum;
}

template <uint32_t total_time_quanta, uint32_t actual_time_quantum,
          uint32_t bitrate_hz>
consteval auto get_actual_bitrate() -> uint32_t {
    constexpr auto actual_bitrate =
        NS_PER_S / (total_time_quanta * actual_time_quantum);
    // note: this logic may look backward but it's because assert fails
    // and prints the message if the predicate is _false_
    static_assert(actual_bitrate < (bitrate_hz + (bitrate_hz >> 3)),
                  "Bitrate >12% too high");
    static_assert(actual_bitrate > (bitrate_hz - (bitrate_hz >> 3)),
                  "Bitrate >12% too low");
    return actual_bitrate;
}

template <uint32_t bitrate_hz, uint32_t time_quantum_ns>
consteval auto get_total_time_quanta() -> uint32_t {
    return NS_PER_S / (bitrate_hz * time_quantum_ns);
}

static constexpr uint16_t SAMPLE_POINT_MAX = 1000;

template <uint32_t total_time_quanta, uint16_t sample_point_ratio>
consteval auto get_segment_1_quanta() -> uint32_t {
    static_assert(sample_point_ratio < 1000,
                  "sample point ratio should be in (1, 1000) and will be "
                  "evaluated as sample_point_ratio / 1000");
    return static_cast<uint32_t>((total_time_quanta * sample_point_ratio) /
                                 SAMPLE_POINT_MAX) -
           1;
}

template <uint32_t total_time_quanta, uint16_t sample_point_ratio>
consteval auto get_segment_2_quanta() -> uint8_t {
    static_assert(sample_point_ratio < 1000,
                  "sample point ratio should be in (1, 1000) and will be "
                  "evaluated as sample_point_ratio / 1000");
    constexpr auto segment_1_quanta =
        get_segment_1_quanta<total_time_quanta, sample_point_ratio>();
    constexpr auto try_segment_2_quanta = static_cast<uint32_t>(
        (total_time_quanta * (SAMPLE_POINT_MAX - sample_point_ratio)) /
        SAMPLE_POINT_MAX);
    return (
        ((segment_1_quanta + try_segment_2_quanta) < (total_time_quanta + 1))
            ? (try_segment_2_quanta + 1)
            : try_segment_2_quanta);
}

/*
** Generate can bit timing configuration values that are compatible with the
** ST HAL based on both device-specific factors and well known CANbus
*parameters.
**
** @tparam can_clock_hz The speed of the clock that drives the CANbus
*peripheral,
** in Hz. Unfortunately, to keep this compile-time computable, it must be
*written
** down directly based on understanding of the clock configuration.
**
** @tparam time_quantum_ns The desired time quantum to configure, in ns. The
*exact
** value may be impossible to achieve based on available clock division factors.
** The actual quantum time used is available as the actual_time_quantum static
** member. Compilation will fail if the actual time quantum is different from
** the requested by more than 12%.
**
** @tparam bitrate_hz The desired bitrate of the CANbus, in hz (or, really,
*baud).
** This may not be achievable based on the exact time quantum calculated; if it
** diverges by more than 12%, compilation will fail. The generated value may be
** accessed from the static member actual_bitrate.
**
** @tparam sample_point_ratio The desired location of the CAN protocol sample
** point. This is in parts-per-thousand, unitless.
**
** @tparam sync_jump_width The value to use for sync jump width, in time quanta.
*/

template <uint32_t can_clock_hz, uint32_t time_quantum_ns, uint32_t bitrate_hz,
          uint16_t sample_point_ratio, uint8_t sync_jump_width = 1>
struct BitTimings {
    constexpr BitTimings() {
        static_cast<void>(actual_bitrate);
        static_cast<void>(actual_time_quantum);
    }
    // The clock divider to set for the time quantum generator.
    static constexpr uint8_t clock_divider =
        get_clock_divider<can_clock_hz, time_quantum_ns>();
    // The actual length of the time quantum based on the input clock and
    // divider, in nanoseconds.
    static constexpr uint32_t actual_time_quantum =
        get_actual_time_quantum<can_clock_hz, clock_divider, time_quantum_ns>();
    // The total number of time quanta in a bit.
    static constexpr uint32_t total_time_quanta =
        get_total_time_quanta<bitrate_hz, actual_time_quantum>();
    // The actual bitrate in Hz based on the time quantum length.
    static constexpr uint32_t actual_bitrate =
        get_actual_bitrate<total_time_quanta, actual_time_quantum,
                           bitrate_hz>();
    // The number of time quanta in protocol propagation segment +
    // protocol segment 1 (the ST hal does not differentiate these two)
    static constexpr uint8_t segment_1_quanta =
        get_segment_1_quanta<total_time_quanta, sample_point_ratio>();
    // The number of time quanta in protocol segment 2
    static constexpr uint8_t segment_2_quanta =
        get_segment_2_quanta<total_time_quanta, sample_point_ratio>();
    // The max allowable sync jump width for clock skew synchronization in
    // time quanta.
    static constexpr uint8_t max_sync_jump_width = sync_jump_width;
};

};  // namespace bit_timings
};  // namespace can
