#pragma once

#include "motor-control/core/brushed_motor/driver_interface.hpp"

namespace sim_brushed_motor_hardware_iface {

using namespace brushed_motor_driver;

class SimBrushedMotorDriverIface : public BrushedMotorDriverIface {
  public:
    bool start_digital_analog_converter() final { return true; }

    bool stop_digital_analog_converter() final { return true; }

    bool set_reference_voltage(float v) final {
        v_ref = v;
        return true;
    }

    void setup() final {}

    void update_pwm_settings(uint32_t) final {}

    auto pwm_active_duty_clamp(uint32_t duty_cycle) -> uint32_t final {
        return std::clamp(duty_cycle, uint32_t(7), uint32_t(100));
    }

    [[nodiscard]] auto get_current_vref() const -> float final { return v_ref; }

    [[nodiscard]] auto get_current_duty_cycle() const -> uint32_t final {
        return duty_cycle;
    }

  private:
    float v_ref = 0;
    uint32_t duty_cycle = 0;
};

}  // namespace sim_brushed_motor_hardware_iface
