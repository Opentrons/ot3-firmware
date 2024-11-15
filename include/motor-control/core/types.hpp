#pragma once

#include <atomic>
#include <cstdint>

#include "can/core/ids.hpp"

using sq0_31 = int32_t;  // 0: signed bit,  1-31: fractional bits
using sq15_16 =
    int32_t;  // 0: signed bit,  1-14: integer bits, 15-31: fractional bits
using q31_31 =
    uint64_t;  // 0: overflow bit, 1-32: integer bits, 33-64: fractional bits

using sq31_31 = int64_t;

using stepper_timer_ticks = uint64_t;
using brushed_timer_ticks = uint64_t;
using steps_per_tick = sq0_31;
using steps_per_tick_sq = sq0_31;

class MotorPositionStatus {
  public:
    using Flags = can::ids::MotorPositionFlags;

    auto set_flag(Flags flag) -> void;

    auto clear_flag(Flags flag) -> void;

    [[nodiscard]] auto check_flag(Flags flag) const -> bool;

    [[nodiscard]] auto get_flags() const -> uint8_t;

  private:
    std::atomic_uint8_t backing{0};
};

struct __attribute__((__packed__)) UsageRequestSet {
    uint16_t eeprom_key;
    uint16_t type_key;
    uint16_t length;
};

enum class BrushedMotorState : uint8_t {
    UNHOMED = 0x0,
    FORCE_CONTROLLING_HOME = 0x1,
    FORCE_CONTROLLING = 0x2,
    POSITION_CONTROLLING = 0x3,
    STOPPED = 0x4
};
