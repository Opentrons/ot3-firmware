#pragma once

#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "spi.hpp"

using namespace motor_driver;
using namespace motion_controller;

namespace motor_class {

template <typename SpiDriver>
requires spi::TMC2130Spi<SpiDriver, spi::BufferSize>
class Motor {
  private:
    SpiDriver& spi_comms;

  public:
    explicit Motor(SpiDriver& spi) : spi_comms(spi) {}
    MotorDriver<SpiDriver> driver = MotorDriver{spi_comms};
    MotionController<SpiDriver> motion_controller = MotionController{spi_comms};
};

}  // namespace motor_class
