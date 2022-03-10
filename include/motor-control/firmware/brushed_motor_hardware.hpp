#pragma once

#include <cstdint>

#include "motor-control/core/motor_hardware_interface.hpp"

namespace motor_hardware {

constexpr float VOLTAGE_REFERENCE = 3.3;
constexpr int DAC_DATA_LENGTH = 12;
constexpr int DAC_DATA_MULTIPLIER = (2 ^ DAC_DATA_LENGTH) - 1;

struct DacConfig {
    void* dac_handle;
    uint32_t channel;
    uint32_t data_algn;
};

struct BrushedHardwareConfig {
    PinConfig pwm_1;
    PinConfig pwm_2;
    PinConfig enable;
    PinConfig limit_switch;
};

class BrushedMotorHardware : public BrushedMotorHardwareIface {
  public:
    ~BrushedMotorHardware() final = default;
    BrushedMotorHardware() = delete;
    BrushedMotorHardware(const BrushedHardwareConfig& config,
                         const DacConfig& dac_config)
        : pins(config), dac(dac_config) {}
    BrushedMotorHardware(const BrushedMotorHardware&) = default;
    auto operator=(const BrushedMotorHardware&)
        -> BrushedMotorHardware& = default;
    BrushedMotorHardware(BrushedMotorHardware&&) = default;
    auto operator=(BrushedMotorHardware&&) -> BrushedMotorHardware& = default;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    auto check_limit_switch() -> bool final;
    auto start_digital_analog_converter() -> bool;
    auto stop_digital_analog_converter() -> bool;
    auto set_reference_voltage(float val) -> bool;

  private:
    BrushedHardwareConfig pins;
    DacConfig dac;
};

};  // namespace motor_hardware
