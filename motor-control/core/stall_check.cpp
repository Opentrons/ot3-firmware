#include "motor-control/core/stall_check.hpp"

#include <cstdlib>

#include "motor-control/core/utils.hpp"

using namespace stall_check;

StallCheck::StallCheck(float encoder_tick_per_um, float stepper_tick_per_um,
                       uint32_t um_threshold)
    : _encoder_tick_per_um{encoder_tick_per_um},
      _stepper_tick_per_um{stepper_tick_per_um},
      _stepper_ticks_to_encoder_ticks_ratio{convert_to_fixed_point_64_bit(
          (encoder_tick_per_um == 0.0F)
              ? 0
              : _stepper_tick_per_um / _encoder_tick_per_um,
          RADIX)},
      _um_threshold{um_threshold},
      _encoder_step_threshold{convert_to_fixed_point_64_bit(
          encoder_tick_per_um * um_threshold, RADIX)},
      _stepper_step_threshold{convert_to_fixed_point_64_bit(
          stepper_tick_per_um * um_threshold, RADIX)} {
    reset_itr_counts(0);
}

auto StallCheck::reset_itr_counts(int32_t stepper_steps) -> void {
    _stepper_counts = stepper_steps * RADIX_SHIFTED;

    _next_threshold_positive = _stepper_counts + _stepper_step_threshold;
    _next_threshold_negative = _stepper_counts - _stepper_step_threshold;

    _encoder_ideal_counts = convert_to_fixed_point_64_bit(
        (_encoder_tick_per_um * stepper_steps) / _stepper_tick_per_um, RADIX);
}

[[nodiscard]] auto StallCheck::step_itr(bool direction) -> bool {
    if (!has_encoder()) {
        return false;
    }
    _stepper_counts += direction ? RADIX_SHIFTED : -RADIX_SHIFTED;
    if (_stepper_counts > _next_threshold_positive) {
        _next_threshold_positive += _stepper_step_threshold;
        _next_threshold_negative += _stepper_step_threshold;
        _encoder_ideal_counts += _encoder_step_threshold;
        return true;
    }
    if (_stepper_counts < _next_threshold_negative) {
        _next_threshold_positive -= _stepper_step_threshold;
        _next_threshold_negative -= _stepper_step_threshold;
        _encoder_ideal_counts -= _encoder_step_threshold;
        return true;
    }
    return false;
}

[[nodiscard]] auto StallCheck::check_stall_itr(int32_t encoder_steps) const
    -> bool {
    if (!has_encoder()) {
        return true;
    }
    sq31_31 diff = std::llabs(_encoder_ideal_counts -
                              (static_cast<sq31_31>(encoder_steps) << RADIX));
    return diff <= _encoder_step_threshold;
}

[[nodiscard]] auto StallCheck::has_encoder() const -> bool {
    return _encoder_tick_per_um != 0.0F;
}

[[nodiscard]] auto StallCheck::encoder_ticks_to_stepper_ticks(
    uint32_t encoder_steps) const -> uint32_t {
    return fixed_point_multiply(_stepper_ticks_to_encoder_ticks_ratio,
                                encoder_steps);
}

[[nodiscard]] auto StallCheck::encoder_um_per_tick() const -> float {
    if (_encoder_tick_per_um == 0) {
        return 0;
    }
    return 1.0F / _encoder_tick_per_um;
}

[[nodiscard]] auto StallCheck::stepper_um_per_tick() const -> float {
    if (_stepper_tick_per_um == 0) {
        return 0;
    }
    return 1.0F / _stepper_tick_per_um;
}
