#pragma once

#include "common/core/logging.h"
#include "motor-control/core/motor_hardware_interface.hpp"

namespace sim_motor_hardware_iface {

class SimMotorHardwareIface : public motor_hardware::StepperMotorHardwareIface {
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
    void set_LED(bool) final {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() final { return true; }
    void reset_encoder_pulses() final { test_pulses = 0; }
    uint32_t get_encoder_pulses() final { return test_pulses; }
    void sim_set_encoder_pulses(uint32_t pulses) { test_pulses = pulses; }
    void clear_encoder_SR() final { sr_flag = false; }
    bool get_encoder_SR_flag() final { return !sr_flag; }
    bool get_encoder_direction() final { return enc_direction; }

  private:
    bool limit_switch_status = false;
    uint32_t test_pulses = 0;
    bool sr_flag = false;
    bool enc_direction = false;
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
    uint32_t get_encoder_pulses() final { return 0; }
    void sim_set_encoder_pulses(uint32_t pulses) { test_pulses = pulses; }

  private:
    bool limit_switch_status = false;
    uint32_t test_pulses = 0;
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
    uint32_t get_encoder_pulses() final { return test_pulses; }
    void sim_set_encoder_pulses(uint32_t pulses) { test_pulses = pulses; }
    void trigger_tip_sense() { tip_sense_status = true; }

  private:
    bool limit_switch_status = false;
    bool tip_sense_status = false;
    uint32_t test_pulses = 0;
};

}  // namespace sim_motor_hardware_iface
