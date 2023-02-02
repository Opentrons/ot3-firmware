#pragma once

#include <concepts>
#include <memory>

#include "common/core/freertos_synchronization.hpp"
#include "common/core/logging.h"
#include "common/simulation/state_manager.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"
#include "motor-control/core/types.hpp"
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

class SimMotorHardwareIface {
  public:
    SimMotorHardwareIface(MoveMessageHardware id)
        : motor_hardware::StepperMotorHardwareIface(), _id(id) {}
    void step() {
        if (_state_manager) {
            _state_manager->send_move_msg(_id, _direction);
        }
        test_pulses += (_direction == Direction::POSITIVE) ? 1 : -1;
    }
    void unstep() {}
    void positive_direction() { _direction = Direction::POSITIVE; }
    void negative_direction() { _direction = Direction::NEGATIVE; }
    void activate_motor() {}
    void deactivate_motor() {}
    void start_timer_interrupt() {}
    void stop_timer_interrupt() {}
    bool check_limit_switch() {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void read_limit_switch() {}
    void read_estop_in() {}
    void read_sync_in() {}
    void set_LED(bool) {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() {
        if (_state_manager) {
            return _state_manager->current_sync_state() == SyncPinState::HIGH;
        }
        return true;
    }
    void reset_encoder_pulses() { test_pulses = 0; }
    int32_t get_encoder_pulses() {
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

    bool check_estop_in() { return estop_detected; }

    void set_estop(bool estop_pressed) { estop_detected = estop_pressed; }

    [[nodiscard]] auto get_step_tracker() const -> uint32_t {
        return step_tracker.load();
    }
    auto reset_step_tracker() -> void { set_step_tracker(0); }
    auto set_step_tracker(uint32_t val) -> void { step_tracker.store(val); }

    MotorPositionStatus position_flags{};

  private:
    bool limit_switch_status = false;
    int32_t test_pulses = 0;
    MoveMessageHardware _id;
    StateManagerHandle _state_manager = nullptr;
    Direction _direction = Direction::POSITIVE;
    float _encoder_ticks_per_pulse = 0;
    bool estop_detected = false;

    // Used to track the position in microsteps.
    std::atomic<uint32_t> step_tracker{0};
};

class SimBrushedMotorHardwareIface {
  public:
    SimBrushedMotorHardwareIface(MoveMessageHardware id)
        : motor_hardware::BrushedMotorHardwareIface(), _id(id) {}
    void positive_direction() {}
    void negative_direction() {}
    void activate_motor() {}
    void deactivate_motor() {}
    void start_timer_interrupt() {}
    void stop_timer_interrupt() {}
    bool check_limit_switch() {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void read_limit_switch() {}
    void read_estop_in() {}
    void read_sync_in() {}
    void trigger_limit_switch() { limit_switch_status = true; }
    void grip() {}
    void ungrip() {}
    void stop_pwm() {}
    bool check_sync_in() {
        if (_state_manager) {
            return _state_manager->current_sync_state() == SyncPinState::HIGH;
        }
        return true;
    }
    void reset_encoder_pulses() { test_pulses = 0; }
    int32_t get_encoder_pulses() { return test_pulses; }
    void set_encoder_pulses(int32_t pulses) { test_pulses = pulses; }
    double update_control(int32_t encoder_error) {
        return controller_loop.compute(encoder_error);
    }
    void reset_control() { controller_loop.reset(); }

    void provide_state_manager(StateManagerHandle handle) {
        _state_manager = handle;
    }

    bool check_estop_in() { return estop_detected; }

    void set_estop(bool estop_pressed) { estop_detected = estop_pressed; }

    void set_stay_enabled(bool state) { stay_enabled = state; }
    auto get_stay_enabled() -> bool { return stay_enabled; }

    MotorPositionStatus position_flags{};

  private:
    bool stay_enabled = false;
    bool limit_switch_status = false;
    int32_t test_pulses = 0;
    // these controller loop values were selected just because testing
    // does not emulate change in speed and these give us pretty good values
    // when the "motor" instantly goes to top speed then instantly stops
    ot_utils::pid::PID controller_loop{0.008,         0.0045, 0.000015,
                                       1.F / 32000.0, 7,      -7};
    StateManagerHandle _state_manager = nullptr;
    MoveMessageHardware _id;
    bool estop_detected = false;
};

class SimGearMotorHardwareIface {
  public:
    void step() {
        test_pulses += (_direction == Direction::POSITIVE) ? 1 : -1;
    }
    void unstep() {}
    void positive_direction() { _direction = Direction::POSITIVE; }
    void negative_direction() { _direction = Direction::NEGATIVE; }
    void activate_motor() {}
    void deactivate_motor() {}
    void start_timer_interrupt() {}
    void stop_timer_interrupt() {}
    bool check_limit_switch() {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    bool check_tip_sense() {
        if (tip_sense_status) {
            tip_sense_status = false;
            return true;
        }
        return false;
    }
    void read_limit_switch() {}
    void read_estop_in() {}
    void read_sync_in() {}
    void read_tip_sense() {}
    void set_LED(bool) {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() {
        if (_state_manager) {
            return _state_manager->current_sync_state() == SyncPinState::HIGH;
        }
        return true;
    }
    void reset_encoder_pulses() { test_pulses = 0; }
    int32_t get_encoder_pulses() {
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

    bool check_estop_in() { return estop_detected; }

    void set_estop(bool estop_pressed) { estop_detected = estop_pressed; }

    MotorPositionStatus position_flags{};

  private:
    bool limit_switch_status = false;
    bool tip_sense_status = false;
    int32_t test_pulses = 0;
    StateManagerHandle _state_manager = nullptr;
    Direction _direction = Direction::POSITIVE;
    float _encoder_ticks_per_pulse = 0;
    bool estop_detected = false;
};

}  // namespace sim_motor_hardware_iface
