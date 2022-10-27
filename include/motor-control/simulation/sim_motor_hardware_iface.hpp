#pragma once

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

class SimMotorHardwareIface : public motor_hardware::StepperMotorHardwareIface {
  public:
    SimMotorHardwareIface(MoveMessageHardware id)
        : motor_hardware::StepperMotorHardwareIface(), _id(id) {}
    void step() final {
        if (_state_manager) {
            _state_manager->send_move_msg(_id, _direction);
        }
    }
    void unstep() final {}
    void positive_direction() final { _direction = Direction::POSITIVE; }
    void negative_direction() final { _direction = Direction::NEGATIVE; }
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool check_limit_switch() final {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void set_LED(bool) final {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() final {
        if (_state_manager) {
            return _state_manager->get_sync_state() == SyncPinState::HIGH;
        }
        return true;
    }
    void reset_encoder_pulses() final { test_pulses = 0; }
    int32_t get_encoder_pulses() final { return test_pulses; }
    void sim_set_encoder_pulses(uint32_t pulses) { test_pulses = pulses; }

    void change_hardware_id(MoveMessageHardware id) { _id = id; }

    void provide_state_manager(StateManagerHandle handle) {
        _state_manager = handle;
    }

  private:
    bool limit_switch_status = false;
    int32_t test_pulses = 0;
    MoveMessageHardware _id;
    StateManagerHandle _state_manager = nullptr;
    Direction _direction = Direction::POSITIVE;
};

class SimBrushedMotorHardwareIface
    : public motor_hardware::BrushedMotorHardwareIface {
  public:
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool check_limit_switch() final {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void grip() final {}
    void ungrip() final {}
    void stop_pwm() final {}
    bool check_sync_in() final { return true; }
    void reset_encoder_pulses() final { test_pulses = 0; }
    int32_t get_encoder_pulses() final { return 0; }
    void sim_set_encoder_pulses(int32_t pulses) { test_pulses = pulses; }

    double update_control(int32_t encoder_error) {
        return controller_loop.compute(encoder_error);
    }
    void reset_control() { controller_loop.reset(); }

  private:
    bool limit_switch_status = false;
    uint32_t test_pulses = 0;
    // these controller loop values were selected just because testing
    // does not emulate change in speed and these give us pretty good values
    // when the "motor" instantly goes to top speed then instantly stops
    ot_utils::pid::PID controller_loop{0.008,         0.0045, 0.000015,
                                       1.F / 32000.0, 7,      -7};
};

class SimGearMotorHardwareIface
    : public motor_hardware::PipetteStepperMotorHardwareIface {
  public:
    void step() final {}
    void unstep() final {}
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
    bool check_limit_switch() final {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    bool check_tip_sense() final {
        if (tip_sense_status) {
            tip_sense_status = false;
            return true;
        }
        return false;
    }
    void set_LED(bool) final {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() final { return true; }
    void reset_encoder_pulses() final { test_pulses = 0; }
    int32_t get_encoder_pulses() final { return test_pulses; }
    void sim_set_encoder_pulses(int32_t pulses) { test_pulses = pulses; }
    void trigger_tip_sense() { tip_sense_status = true; }

  private:
    bool limit_switch_status = false;
    bool tip_sense_status = false;
    int32_t test_pulses = 0;
};

}  // namespace sim_motor_hardware_iface
