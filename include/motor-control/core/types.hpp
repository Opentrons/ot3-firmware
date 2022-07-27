#pragma once

#include <atomic>
#include <cstdint>
#include <can/core/ids.hpp>

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

// This should be consteval but if it is then the compiler crashes in anything up to
// and including 12.3. I think it is fixed by
// https://gcc.gnu.org/git/?p=gcc.git;a=commit;h=647537adefb34041cc2d44585252fd765cc0daae
auto operator|(can::ids::PositionFlags a, can::ids::PositionFlags b) -> uint32_t {
    return static_cast<uint32_t>(a) | static_cast<uint32_t>(b);
}

struct PositionStatus {
    // Flags with the bit positions of can::ids::PositionFlags
    std::atomic<uint32_t> flags{0};
    /*
    ** Performs flags |= flags_to_set, but atomically.
    */
    auto set_flags(uint32_t flags_to_set) -> void {
        std::atomic_fetch_or(&flags, flags_to_set);
    }
    /*
    ** Performs flags &= ~flags_to_set, but atomically
    */
    auto clear_flags(uint32_t flags_to_clear) -> void {
        std::atomic_fetch_and(&flags, ~flags_to_clear);
    }
};

struct StepperPositionStatus : public PositionStatus {
    std::atomic<uint32_t> stepper_position_steps{0};
    auto update_stepper_position(uint32_t new_position) -> void {
        stepper_position_steps = new_position;
    }
};

struct StepperWithEncoderPositionStatus : public StepperPositionStatus {
    std::atomic<int32_t> encoder_position_steps{0};
    auto update_encoder_position(int32_t new_position) {
        encoder_position_steps = new_position;
    }
};

struct EncoderOnlyPositionStatus : public PositionStatus {
    std::atomic<int32_t> encoder_position_steps{0};
    auto update_encoder_position(int32_t new_position) -> void {
        encoder_position_steps = new_position;
    }
};

template <typename Client>
concept PositionStatusClient = requires(Client client, uint32_t flag_updates) {
    {client.set_position_flags(flag_updates)};
    {client.clear_position_flags(flag_updates)};
};

template <typename Client>
concept StepperPositionOnlyClient = requires(Client client,
                                             uint32_t stepper_position) {
    {client.update_stepper_position(stepper_position)};
};

template <typename Client>
concept EncoderPositionOnlyClient = requires(Client client,
                                             int32_t encoder_position) {
    {client.update_encoder_position(encoder_position)};
};

template <typename Client>
concept StepperPositionStatusClient =
    PositionStatusClient<Client> && StepperPositionOnlyClient<Client>;

template <typename Client>
concept StepperWithEncoderPositionStatusClient =
    StepperPositionStatusClient<Client> && EncoderPositionOnlyClient<Client>;

template <typename Client>
concept EncoderOnlyPositionStatusClient =
    PositionStatusClient<Client> && EncoderPositionOnlyClient<Client>;
