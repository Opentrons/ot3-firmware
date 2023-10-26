#pragma once

#include <atomic>
#include <concepts>
#include <memory>

#include "common/core/freertos_synchronization.hpp"
#include "common/core/logging.h"
#include "common/simulation/state_manager.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "ot_utils/core/pid.hpp"

namespace sim_motor_hardware_iface {

using StateManager = state_manager::StateManagerConnection<
    freertos_synchronization::FreeRTOSCriticalSection>;
using StateManagerHandle = std::shared_ptr<StateManager>;

template <typename MSC>
concept MotionSystemConfig = requires(MSC msc) {
    std::is_arithmetic_v<decltype(msc.steps_per_rev)>
        &&std::is_arithmetic_v<decltype(msc.microstep)>
            &&std::is_arithmetic_v<decltype(msc.encoder_pulses_per_rev)>;
};

class SimMotorHardwareIface : public motor_hardware::StepperMotorHardwareIface {
  public:
    SimMotorHardwareIface(MoveMessageHardware id)
        : motor_hardware::StepperMotorHardwareIface(), _id(id) {}
    void step() final {
        if (_state_manager) {
            _state_manager->send_move_msg(_id, _direction);
        }
        test_pulses += (_direction == Direction::POSITIVE) ? 1 : -1;
    }
    void unstep() final {}
    void positive_direction() final { _direction = Direction::POSITIVE; }
    void negative_direction() final { _direction = Direction::NEGATIVE; }
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool is_timer_interrupt_running() final { return true; }
    bool check_limit_switch() final {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void read_limit_switch() final {}
    void read_estop_in() final {}
    void read_sync_in() final {}
    void set_LED(bool) final {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() final {
        if (_state_manager) {
            return _state_manager->current_sync_state() == SyncPinState::HIGH;
        }
        return true;
    }
    void reset_encoder_pulses() final { test_pulses = 0; }
    int32_t get_encoder_pulses() final {
        return static_cast<int32_t>(test_pulses * _encoder_ticks_per_pulse);
    }

    void change_hardware_id(MoveMessageHardware id) { _id = id; }

    void provide_state_manager(StateManagerHandle handle) {
        _state_manager = handle;
    }

    template <MotionSystemConfig MSC>
    void provide_mech_config(const MSC &config) {
        // Exists to match the firmware's interrupt scheme
        static constexpr float encoder_extra_factor = 4.0;
        if (config.steps_per_rev == 0 || config.microstep == 0) {
            _encoder_ticks_per_pulse = 0;
        } else {
            _encoder_ticks_per_pulse =
                static_cast<float>(config.encoder_pulses_per_rev *
                                   encoder_extra_factor) /
                (config.steps_per_rev * config.microstep);
        }
    }

    bool check_estop_in() final { return estop_detected; }

    void set_estop(bool estop_pressed) { estop_detected = estop_pressed; }
    auto has_cancel_request() -> uint8_t final {
        return cancel_request.exchange(0);
    }
    void request_cancel(uint8_t error_severity) final {
        cancel_request.store(error_severity);
    }
    auto get_usage_eeprom_config()
        -> motor_hardware::UsageEEpromConfig & final {
        return eeprom_config;
    }
    void disable_encoder() final {}
    void enable_encoder() final {}

  private:
    bool limit_switch_status = false;
    int32_t test_pulses = 0;
    MoveMessageHardware _id;
    StateManagerHandle _state_manager = nullptr;
    Direction _direction = Direction::POSITIVE;
    float _encoder_ticks_per_pulse = 0;
    bool estop_detected = false;
    std::atomic<uint8_t> cancel_request = 0;
    motor_hardware::UsageEEpromConfig eeprom_config = {
        std::array<UsageRequestSet, 1>{UsageRequestSet{
            .eeprom_key = 0,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len}}};
};

class SimBrushedMotorHardwareIface
    : public motor_hardware::BrushedMotorHardwareIface {
  public:
    SimBrushedMotorHardwareIface(MoveMessageHardware id)
        : motor_hardware::BrushedMotorHardwareIface(), _id(id) {}
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool is_timer_interrupt_running() final { return true; }
    bool check_limit_switch() final {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void read_limit_switch() final {}
    void read_estop_in() final {}
    void read_sync_in() final {}
    void trigger_limit_switch() { limit_switch_status = true; }
    void grip() final {}
    void ungrip() final {}
    void stop_pwm() final {}
    bool check_sync_in() final {
        if (_state_manager) {
            return _state_manager->current_sync_state() == SyncPinState::HIGH;
        }
        return true;
    }
    void reset_encoder_pulses() final { test_pulses = 0; }
    int32_t get_encoder_pulses() final { return test_pulses; }
    void set_encoder_pulses(int32_t pulses) { test_pulses = pulses; }
    double update_control(int32_t encoder_error) {
        return controller_loop.compute(encoder_error);
    }
    void reset_control() { controller_loop.reset(); }

    void provide_state_manager(StateManagerHandle handle) {
        _state_manager = handle;
    }

    bool check_estop_in() final { return estop_detected; }

    void set_estop(bool estop_pressed) { estop_detected = estop_pressed; }

    void set_stay_enabled(bool state) final { stay_enabled = state; }
    auto get_stay_enabled() -> bool final { return stay_enabled; }
    auto get_usage_eeprom_config()
        -> motor_hardware::UsageEEpromConfig & final {
        return eeprom_config;
    }
    auto get_stopwatch_pulses(bool clear) -> uint16_t {
        auto ret = stopwatch_pulses;
        if (clear) {
            stopwatch_pulses = 0;
        }
        return ret;
    }

    auto has_cancel_request() -> uint8_t final {
        return cancel_request.exchange(0);
    }
    void request_cancel(uint8_t error_severity) final {
        cancel_request.store(error_severity);
    }

    void disable_encoder() final {}
    void enable_encoder() final {}

    void set_motor_state(BrushedMotorState state) final { motor_state = state; }
    auto get_motor_state() -> BrushedMotorState final { return motor_state; }

  private:
    bool stay_enabled = false;
    bool limit_switch_status = false;
    int32_t test_pulses = 0;
    int16_t stopwatch_pulses = 0;
    // these controller loop values were selected just because testing
    // does not emulate change in speed and these give us pretty good values
    // when the "motor" instantly goes to top speed then instantly stops
    ot_utils::pid::PID controller_loop{0.008,         0.0045, 0.000015,
                                       1.F / 32000.0, 7,      -7};
    StateManagerHandle _state_manager = nullptr;
    MoveMessageHardware _id;
    bool estop_detected = false;
    std::atomic<uint8_t> cancel_request = 0;
    BrushedMotorState motor_state = BrushedMotorState::UNHOMED;
    motor_hardware::UsageEEpromConfig eeprom_config{
        std::array<UsageRequestSet, 1>{UsageRequestSet{
            .eeprom_key = 0,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len}}};
};

class SimGearMotorHardwareIface
    : public motor_hardware::StepperMotorHardwareIface {
  public:
    void step() final {
        test_pulses += (_direction == Direction::POSITIVE) ? 1 : -1;
    }
    void unstep() final {}
    void positive_direction() final { _direction = Direction::POSITIVE; }
    void negative_direction() final { _direction = Direction::NEGATIVE; }
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool is_timer_interrupt_running() final { return true; }
    bool check_limit_switch() final {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void read_limit_switch() final {}
    void read_estop_in() final {}
    void read_sync_in() final {}
    void set_LED(bool) final {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() final {
        if (_state_manager) {
            return _state_manager->current_sync_state() == SyncPinState::HIGH;
        }
        return true;
    }
    void reset_encoder_pulses() final { test_pulses = 0; }
    int32_t get_encoder_pulses() final {
        return static_cast<int32_t>(test_pulses * _encoder_ticks_per_pulse);
    }
    void trigger_tip_sense() { tip_sense_status = true; }

    void provide_state_manager(StateManagerHandle handle) {
        _state_manager = handle;
    }

    template <MotionSystemConfig MSC>
    void provide_mech_config(const MSC &config) {
        if (config.steps_per_rev == 0 || config.microstep == 0) {
            _encoder_ticks_per_pulse = 0;
        } else {
            _encoder_ticks_per_pulse =
                config.encoder_pulses_per_rev /
                (config.steps_per_rev * config.microstep);
        }
    }

    bool check_estop_in() final { return estop_detected; }

    void set_estop(bool estop_pressed) { estop_detected = estop_pressed; }
    auto has_cancel_request() -> uint8_t final {
        return cancel_request.exchange(0);
    }
    void request_cancel(uint8_t error_severity) final {
        cancel_request.store(error_severity);
    }

    auto get_usage_eeprom_config()
        -> motor_hardware::UsageEEpromConfig & final {
        return eeprom_config;
    }
    void disable_encoder() final {}
    void enable_encoder() final {}

  private:
    bool limit_switch_status = false;
    bool tip_sense_status = false;
    int32_t test_pulses = 0;
    StateManagerHandle _state_manager = nullptr;
    Direction _direction = Direction::POSITIVE;
    float _encoder_ticks_per_pulse = 0;
    bool estop_detected = false;
    std::atomic<uint8_t> cancel_request = 0;
    motor_hardware::UsageEEpromConfig eeprom_config = {
        std::array<UsageRequestSet, 1>{UsageRequestSet{
            .eeprom_key = 0,
            .type_key =
                uint16_t(can::ids::MotorUsageValueType::linear_motor_distance),
            .length = usage_storage_task::distance_data_usage_len}}};
};

}  // namespace sim_motor_hardware_iface
