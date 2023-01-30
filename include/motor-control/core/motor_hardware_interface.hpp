#pragma once

#include <atomic>
#include <cstdint>
#include <utility>
#include <concepts>

#include "motor-control/core/types.hpp"

namespace motor_hardware {

template<class T>
concept BaseMotorHardware = requires(T t) {
    { t.positive_direction() } -> std::same_as<int>;
    { t.negative_direction() } -> std::same_as<int>;
    { t.activate_motor() } -> std::same_as<int>;
    { t.deactivate_motor() } -> std::same_as<int>;
    { t.check_limit_switch() } -> std::same_as<bool>;
    { t.check_estop_in() } -> std::same_as<bool>;
    { t.check_sync_in() } -> std::same_as<bool>;
    { t.read_limit_switch() } -> std::same_as<int>;
    { t.read_estop_in() } -> std::same_as<int>;
    { t.read_sync_in() } -> std::same_as<int>;
    { t.get_encoder_pulses() } -> std::same_as<int32_t>;
    { t.reset_encoder_pulses() } -> std::same_as<int>;
    { t.start_timer_interrupt() } -> std::same_as<int>;
    { t.stop_timer_interrupt() } -> std::same_as<int>;
    { T::position_flags } -> std::convertible_to<MotorPositionStatus>;
};

template<class T, class U>
concept StepperMotorHardware = requires(T t, U u) {
  {std::is_base_of<U, T>::value };
  { t.step() } -> std::same_as<int>;
  { t.unstep() } -> std::same_as<int>;
  { t.set_LED(bool status) } -> std::same_as<int>;
  { t.get_step_tracker() } -> std::same_as<uint32_t>;
  { t.reset_step_tracker() } -> std::same_as<int16_t>;
  { t.set_step_tracker(uint32_t) } -> std::same_as<int>

}

template<class T, class U>
concept BrushedMotorHardwareIface = requires(T t, U u) {
  {std::is_base_of<U, T>::value };
  { t.grip() } -> std::same_as<int>;
  { t.ungrip() } -> std::same_as<int>;
  { t.stop_pwm() } -> std::same_as<int>;
  { t.update_control(int32_t encoder_error) } -> std::same_as<double>;
  { t.reset_control() } -> std::same_as<int>;
  { t.set_stay_enabled(bool state) } -> std::same_as<int>;
  { t.get_stay_enabled() -> bool } -> std::same_as<int>;
}

template<class T, class U>
concept PipetteStepperMotorHardwareIface = requires(T t, U u) {
  {std::is_base_of<U, T>::value };
  { t.check_tip_sense() } -> std::same_as<bool>;
  { t.read_tip_sense() } -> std::is_void<int>;
}

}  // namespace motor_hardware
