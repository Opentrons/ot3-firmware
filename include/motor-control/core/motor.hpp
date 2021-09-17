#pragma once

#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "spi.hpp"

using namespace motor_driver;
using namespace motion_controller;

namespace motor_class {

template <spi::TMC2130Spi SpiDriver>
struct Motor {
    explicit Motor(SpiDriver& spi, HardwareConfig& config)
        : spi_comms(spi), hardware_config(config) {}
    SpiDriver& spi_comms;
    HardwareConfig& hardware_config;
    MotorDriver<SpiDriver> driver = MotorDriver{spi_comms};
    MotionController<SpiDriver> motion_controller =
        MotionController{spi_comms, hardware_config};
};

}  // namespace motor_class
