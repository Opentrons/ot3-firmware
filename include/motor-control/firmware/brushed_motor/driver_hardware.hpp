#pragma once

#include <cstdint>

#include "motor-control/core/brushed_motor/driver_interface.hpp"

namespace brushed_motor_driver {

struct DacConfig {
    void* dac_handle;
    uint32_t channel;
    uint32_t data_algn;
};

struct DriverConfig {
    float vref;
};

class BrushedMotorDriver : public BrushedMotorDriverIface {
  public:
    ~BrushedMotorDriver() final = default;
    BrushedMotorDriver() = delete;
    BrushedMotorDriver(const DacConfig& dac_config,
                       const DriverConfig& driver_config)
        : dac(dac_config), conf(driver_config) {}
    BrushedMotorDriver(const BrushedMotorDriver&) = default;
    auto operator=(const BrushedMotorDriver&) -> BrushedMotorDriver& = default;
    BrushedMotorDriver(BrushedMotorDriver&&) = default;
    auto operator=(BrushedMotorDriver&&) -> BrushedMotorDriver& = default;

    auto start_digital_analog_converter() -> bool final;
    auto stop_digital_analog_converter() -> bool final;
    auto set_reference_voltage(float val) -> bool final;
    void setup() final;

  private:
    DacConfig dac;
    DriverConfig conf;
};

};  // namespace brushed_motor_driver
