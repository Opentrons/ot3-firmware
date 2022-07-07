#pragma once

#include <cstdint>
#include <functional>

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
    using Callback = std::function<void(uint32_t duty_cycle)>;
    ~BrushedMotorDriver() final = default;
    BrushedMotorDriver() = delete;
    BrushedMotorDriver(const DacConfig& dac_config,
                       const DriverConfig& driver_config, Callback callback)
        : dac(dac_config), conf(driver_config), callback{std::move(callback)} {}
    BrushedMotorDriver(const BrushedMotorDriver&) = default;
    auto operator=(const BrushedMotorDriver&) -> BrushedMotorDriver& = default;
    BrushedMotorDriver(BrushedMotorDriver&&) = default;
    auto operator=(BrushedMotorDriver&&) -> BrushedMotorDriver& = default;

    auto start_digital_analog_converter() -> bool final;
    auto stop_digital_analog_converter() -> bool final;
    auto set_reference_voltage(float val) -> bool final;
    void setup() final;
    void update_pwm_settings(uint32_t duty_cycle) final {
        callback(duty_cycle);
    }

  private:
    DacConfig dac;
    DriverConfig conf;
    Callback callback;
};

};  // namespace brushed_motor_driver
