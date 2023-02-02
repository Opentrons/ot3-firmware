#pragma once

#include <atomic>
#include <cstdint>
#include <utility>
#include <concepts>

namespace motor_hardware {

template<class T>
concept BaseMotorHardwareIface = requires(T t) {
    { t.positive_direction() } -> std::same_as<void>;
    { t.negative_direction() } -> std::same_as<void>;
    { t.activate_motor() } -> std::same_as<void>;
    { t.deactivate_motor() } -> std::same_as<void>;
    { t.check_limit_switch() } -> std::same_as<bool>;
    { t.check_estop_in() } -> std::same_as<bool>;
    { t.check_sync_in() } -> std::same_as<bool>;
    { t.read_limit_switch() } -> std::same_as<void>;
    { t.read_estop_in() } -> std::same_as<void>;
    { t.read_sync_in() } -> std::same_as<void>;
    { t.get_encoder_pulses() } -> std::same_as<int32_t>;
    { t.reset_encoder_pulses() } -> std::same_as<void>;
    { t.start_timer_interrupt() } -> std::same_as<void>;
    { t.stop_timer_interrupt() } -> std::same_as<void>;
    // { t.position_flags } -> MotorPositionStatus;
};

template<class T>
concept StepperMotorHardwareIface = BaseMotorHardwareIface<T> && requires(T t) {
  { t.step() } -> std::same_as<void>;
  { t.unstep() } -> std::same_as<void>;
  { t.set_LED(std::declval<bool>()) } -> std::same_as<void>;
  { t.get_step_tracker() } -> std::same_as<uint32_t>;
  { t.reset_step_tracker() } -> std::same_as<void>;
  { t.set_step_tracker(std::declval<uint32_t>()) } -> std::same_as<void>;
};

template<class T>
concept BrushedMotorHardwareIface = BaseMotorHardwareIface<T> && requires(T t) {
  { t.grip() } -> std::same_as<void>;
  { t.ungrip() } -> std::same_as<void>;
  { t.stop_pwm() } -> std::same_as<void>;
  { t.update_control(std::declval<int32_t>()) } -> std::same_as<double>;
  { t.reset_control() } -> std::same_as<void>;
  { t.set_stay_enabled(std::declval<bool>()) } -> std::same_as<void>;
  { t.get_stay_enabled() } -> std::same_as<bool>;
};

template<class T>
concept PipetteStepperMotorHardwareIface = StepperMotorHardwareIface<T> && requires(T t) {
  { t.check_tip_sense() } -> std::same_as<bool>;
  { t.read_tip_sense() } -> std::same_as<int>;
};

}  // namespace motor_hardware
