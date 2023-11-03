#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "motor-control/core/types.hpp"

namespace motor_hardware {

static constexpr auto max_requests_per_can_message = 5;
class UsageEEpromConfig {
  public:
    template <size_t N>
    UsageEEpromConfig(const std::array<UsageRequestSet, N>& requests) {
        auto i = 0;
        for (auto r : requests) {
            if (i == max_requests_per_can_message) {
                break;
            }
            usage_requests.at(i) = r;
            i++;
        }
        num_keys = i;
    }

    [[nodiscard]] auto get_distance_key() const -> uint16_t {
        for (auto i : usage_requests) {
            if (i.type_key ==
                uint16_t(
                    can::ids::MotorUsageValueType::linear_motor_distance)) {
                return i.eeprom_key;
            }
        }
        return 0xFFFF;
    }

    [[nodiscard]] auto get_gear_distance_key() const -> uint16_t {
        for (auto i : usage_requests) {
            if (i.type_key == uint16_t(can::ids::MotorUsageValueType::
                                           left_gear_motor_distance) ||
                i.type_key == uint16_t(can::ids::MotorUsageValueType::
                                           right_gear_motor_distance)) {
                return i.eeprom_key;
            }
        }
        return 0xFFFF;
    }

    [[nodiscard]] auto get_force_application_time_key() const -> uint16_t {
        for (auto i : usage_requests) {
            if (i.type_key ==
                uint16_t(
                    can::ids::MotorUsageValueType::force_application_time)) {
                return i.eeprom_key;
            }
        }
        return 0xFFFF;
    }
    [[nodiscard]] auto get_error_count_key() const -> uint16_t {
        for (auto i : usage_requests) {
            if (i.type_key ==
                uint16_t(can::ids::MotorUsageValueType::total_error_count)) {
                return i.eeprom_key;
            }
        }
        return 0xFFFF;
    }
    std::array<UsageRequestSet, max_requests_per_can_message> usage_requests{};
    size_t num_keys = 0;
};

// std::optional usage? See HardwareConfig struct
struct __attribute__((packed)) CancelRequest {
    uint8_t severity;
    uint8_t code;
};

class MotorHardwareIface {
  public:
    MotorHardwareIface() = default;
    MotorHardwareIface(const MotorHardwareIface&) = delete;
    MotorHardwareIface(MotorHardwareIface&&) = delete;
    auto operator=(MotorHardwareIface&&) -> MotorHardwareIface& = delete;
    auto operator=(const MotorHardwareIface&) -> MotorHardwareIface& = delete;
    virtual ~MotorHardwareIface() = default;
    virtual void positive_direction() = 0;
    virtual void negative_direction() = 0;
    virtual void activate_motor() = 0;
    virtual void deactivate_motor() = 0;
    virtual auto check_limit_switch() -> bool = 0;
    virtual auto check_estop_in() -> bool = 0;
    virtual auto check_sync_in() -> bool = 0;
    virtual void read_limit_switch() = 0;
    virtual void read_estop_in() = 0;
    virtual void read_sync_in() = 0;
    virtual bool read_tmc_diag0() = 0;
    virtual auto get_encoder_pulses() -> int32_t = 0;
    virtual void reset_encoder_pulses() = 0;
    virtual void start_timer_interrupt() = 0;
    virtual void stop_timer_interrupt() = 0;
    virtual auto is_timer_interrupt_running() -> bool = 0;
    virtual void enable_encoder() = 0;
    virtual void disable_encoder() = 0;

    virtual auto get_cancel_request() -> CancelRequest = 0;
    virtual void set_cancel_request(can::ids::ErrorSeverity error_severity,
                                    can::ids::ErrorCode error_code) = 0;
    virtual void clear_cancel_request() = 0;
    virtual auto get_usage_eeprom_config() -> const UsageEEpromConfig& = 0;

    // This variable can remain public because the only public methods
    // to it are thread-safe anyways.
    MotorPositionStatus position_flags{};
};

class StepperMotorHardwareIface : virtual public MotorHardwareIface {
  public:
    virtual void step() = 0;
    virtual void unstep() = 0;
    virtual void set_LED(bool status) = 0;

    // Position tracker interface is the same for all steppers

    /**
     * @brief Get the current position tracker atomically
     */
    [[nodiscard]] auto get_step_tracker() const -> uint32_t;

    /**
     * @brief Reset the position tracker to 0
     */
    auto reset_step_tracker() -> void;

    /**
     * @brief Set the position tracker to a specified value
     */
    auto set_step_tracker(uint32_t) -> void;

  private:
    // Used to track the position in microsteps.
    std::atomic<uint32_t> step_tracker{0};
};

class BrushedMotorHardwareIface : virtual public MotorHardwareIface {
  public:
    virtual void grip() = 0;
    virtual void ungrip() = 0;
    virtual void stop_pwm() = 0;
    virtual auto update_control(int32_t encoder_error) -> double = 0;
    virtual void reset_control() = 0;
    virtual void set_stay_enabled(bool state) = 0;
    virtual auto get_stay_enabled() -> bool = 0;
    virtual auto get_stopwatch_pulses(bool clear) -> uint16_t = 0;
    virtual void set_motor_state(BrushedMotorState state) = 0;
    virtual auto get_motor_state() -> BrushedMotorState = 0;
};
};  // namespace motor_hardware
