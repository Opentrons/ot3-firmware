#pragma once

#include "drive_train_system.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "spi.hpp"

using namespace motor_driver;
using namespace motion_controller;

namespace motor_class {

template <spi::TMC2130Spi SpiDriver, LinearMechanicalConfig MEConfig>
struct Motor {
    explicit Motor(SpiDriver& spi, MEConfig& me_config, HardwareConfig& config)
        : spi_comms(spi), me_config(me_config), hardware_config(config) {}
    SpiDriver& spi_comms;
    MEConfig& me_config;
    HardwareConfig& hardware_config;
    MotorDriver<SpiDriver> driver = MotorDriver{spi_comms};
    MotionController<SpiDriver, MEConfig> motion_controller =
        MotionController{spi_comms, me_config, hardware_config};
};

}  // namespace motor_class
