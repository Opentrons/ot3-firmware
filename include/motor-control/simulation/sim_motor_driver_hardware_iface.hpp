#pragma once

#include "motor-control/core/brushed_motor/driver_interface.hpp"

namespace sim_brushed_motor_hardware_iface {

using namespace brushed_motor_driver;

class SimBrushedMotorDriverIface : public BrushedMotorDriverIface {
  public:
    bool start_digital_analog_converter() final { return true; }
    bool stop_digital_analog_converter() final { return true; }
    bool set_reference_voltage(float) final { return true; }
    void setup() final {}
    void update_pwm_settings(uint32_t) final {}
};

}  // namespace sim_brushed_motor_hardware_iface
