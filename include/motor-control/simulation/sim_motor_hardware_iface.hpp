#pragma once

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
    void set_LED(bool status) final {}
    void trigger_limit_switch() { limit_switch_status = true; }
    bool check_sync_in() final { return true; }
    void reset_encoder_pulses() final {}
    uint32_t get_encoder_pulses() { return 0; }

  private:
    bool limit_switch_status = false;
};

class SimBrushedMotorHardwareIface
    : public motor_hardware::BrushedMotorHardwareIface {
  public:
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    bool check_limit_switch() final {
        if (limit_switch_status) {
            limit_switch_status = false;
            return true;
        }
        return false;
    }
    void grip() final{};
    void home() final{};
    bool check_sync_in() final { return true; }
    void reset_encoder_pulses() final {}
    uint32_t get_encoder_pulses() { return 0; }

  private:
    bool limit_switch_status = false;
};

}  // namespace sim_motor_hardware_iface
