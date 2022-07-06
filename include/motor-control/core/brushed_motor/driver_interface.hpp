#pragma once

#include <cstdint>

namespace brushed_motor_driver {

constexpr float VOLTAGE_REFERENCE = 3.3;
constexpr int DAC_DATA_LENGTH = 12;
constexpr int DAC_DATA_MULTIPLIER = 2 << (DAC_DATA_LENGTH - 1);

class BrushedMotorDriverIface {
  public:
    BrushedMotorDriverIface() = default;
    BrushedMotorDriverIface(const BrushedMotorDriverIface&) = default;
    BrushedMotorDriverIface(BrushedMotorDriverIface&&) = default;
    auto operator=(BrushedMotorDriverIface&&)
        -> BrushedMotorDriverIface& = default;
    auto operator=(const BrushedMotorDriverIface&)
        -> BrushedMotorDriverIface& = default;
    virtual ~BrushedMotorDriverIface() = default;

    virtual auto start_digital_analog_converter() -> bool = 0;
    virtual auto stop_digital_analog_converter() -> bool = 0;
    virtual auto set_reference_voltage(float val) -> bool = 0;
    virtual void setup() = 0;
    virtual void update_pwm_settings(uint32_t duty_cycle) = 0;
};

}  // namespace brushed_motor_driver
