#pragma once

#include "motor-control/core/motor_hardware_interface.hpp"

namespace sim_motor_hardware_iface {
class SimMotorHardwareIface: public motor_hardware::MotorHardwareIface {

  public:
    void step() final {}
    void unstep() final {}
    void positive_direction() final {}
    void negative_direction() final {}
    void activate_motor() final {}
    void deactivate_motor() final {}
    void start_timer_interrupt() final {}
    void stop_timer_interrupt() final {}
};

}