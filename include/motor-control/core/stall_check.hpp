/**
 * @file stall_check.hpp
 * @brief Class interface for encoder stall detection
 *
 * @details
 */

#pragma once

#include "motor-control/core/types.hpp"

namespace stall_check {

class StallCheck {
  public:
    /**
     * @brief Construct a new Stall Check object
     *
     * @param encoder_tick_per_um Ratio of ticks : µm for encoder
     * @param stepper_tick_per_um Ratio of ticks : µm for microsteps
     * @param um_threshold If encoder and stepper position differs by
     * more than this, consider the stepper stalled.
     */
    StallCheck(float encoder_tick_per_um, float stepper_tick_per_um,
               uint32_t um_threshold);

    /**
     * @brief Reset the counters for the stall check to match the current
     * stepper position.
     * @note This function is NOT optimized for ISR use. It uses floating
     * point math and should only be called from a task context.
     *
     * @param stepper_steps Current microsteps.
     */
    auto reset_itr_counts(int32_t stepper_steps) -> void;

    /**
     * Should be be called whenever the motor interrupt steps the motor.
     * If this function returns true, `check_itr` should be called.
     *
     * @param[in] direction true for a positive step, false for a negative
     * step. Only one step may be taken at a time.
     */
    [[nodiscard]] auto step_itr(bool direction) -> bool
        __attribute__((optimize(3)));

    /**
     * @brief Check the stall status of the motor. This function is optimized
     * to be run in the high-frequency motor interrupt.
     *
     * @param encoder_steps Current microstep position of the encoder.
     * @return True if the motor position is OK, false if the encoder
     * indicates a stall occurred.
     */
    [[nodiscard]] auto check_stall_itr(int32_t encoder_steps) const -> bool
        __attribute__((optimize(3)));

    [[nodiscard]] auto has_encoder() const -> bool;

  private:
    [[nodiscard]] auto encoder_um_per_tick() const -> float;
    [[nodiscard]] auto stepper_um_per_tick() const -> float;

    static constexpr uint64_t RADIX = 31;
    static constexpr uint64_t RADIX_SHIFTED = static_cast<uint64_t>(1) << RADIX;

    // Fixed point representations of the hardware limits
    const float _encoder_tick_per_um;
    const float _stepper_tick_per_um;

    // Stall threshold
    const uint32_t _um_threshold;

    // Number of ticks within the stall thresholds
    const sq31_31 _encoder_step_threshold;
    const sq31_31 _stepper_step_threshold;

    // This is an IDEAL position for the encoder, updated to match
    // the stepper as it increments.
    sq31_31 _encoder_ideal_counts = 0;
    // Running counter for the stepper. This is a fixed point number
    // to prevent error buildup as the magnitude increases over time.
    sq31_31 _stepper_counts = 0;

    sq31_31 _next_threshold_positive = 0;
    sq31_31 _next_threshold_negative = 0;
};

}  // namespace stall_check