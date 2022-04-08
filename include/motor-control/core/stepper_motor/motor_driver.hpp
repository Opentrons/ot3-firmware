#pragma once

#include "common/core/spi.hpp"
#include "motor_driver_config.hpp"
#include "tmc2130.hpp"

namespace motor_driver {

using namespace motor_driver_config;

/*
 * MotorDriver uses SPI communication to initialize, send and receive status and
 * data to and from the TMC2130 motor driver.
 */
class MotorDriver {
  public:
    spi::SpiDeviceBase::BufferType rxBuffer{0};

    explicit MotorDriver(spi::SpiDeviceBase& spi,
                         tmc2130::TMC2130DriverConfig conf)
        : tmc2130{conf, spi} {}

    void setup() { tmc2130.write_config(); }

    auto read(tmc2130::Registers motor_reg, uint32_t command_data) -> uint32_t {
        return tmc2130.read(motor_reg, command_data);
    }

    auto write(tmc2130::Registers motor_reg, uint32_t command_data) -> bool {
        return tmc2130.write(motor_reg, command_data);
    }

    tmc2130::TMC2130 tmc2130;
};

}  // namespace motor_driver
