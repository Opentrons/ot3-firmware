#pragma once

#include <cstdint>

namespace can {
namespace bit_timings {

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
    static_assert(actual_bitrate > (bitrate_hz + (bitrate_hz >> 3)),
                  "Bitrate >12% too high");
    static_assert(actual_bitrate < (bitrate_hz - (bitrate_hz >> 3)),
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

template <uint32_t can_clock_hz, uint32_t time_quantum_ns, uint32_t bitrate_hz,
          uint16_t sample_point_ratio>
struct BitTimings {
    static constexpr uint8_t clock_divider =
        get_clock_divider<can_clock_hz, time_quantum_ns>();
    static constexpr uint32_t actual_time_quantum =
        get_actual_time_quantum<can_clock_hz, clock_divider, time_quantum_ns>();
    static constexpr uint32_t total_time_quanta =
        get_total_time_quanta<bitrate_hz, actual_time_quantum>();
    static constexpr uint32_t actual_bitrate =
        get_actual_bitrate<total_time_quanta, actual_time_quantum,
                           bitrate_hz>();
    static constexpr uint8_t segment_1_quanta =
        get_segment_1_quanta<total_time_quanta, sample_point_ratio>();
    static constexpr uint8_t segment_2_quanta =
        get_segment_2_quanta<total_time_quanta, sample_point_ratio>();
    static constexpr uint8_t max_sync_jump_width = segment_2_quanta;
};

};  // namespace bit_timings
};  // namespace can
