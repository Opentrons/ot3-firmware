#pragma once

#include "linear_motion_system.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "spi.hpp"

using namespace motor_driver;
using namespace motion_controller;
using namespace lms;

namespace motor_class {

template <spi::TMC2130Spi SpiDriver, lms::LMSConfig LMSConf>
struct Motor {
    explicit Motor(SpiDriver& spi, LMSConf& lms_config, HardwareConfig& config)
        : spi_comms(spi),
          linear_motion_sys_config(lms_config),
          hardware_config(config) {}
    SpiDriver& spi_comms;
    LMSConf& linear_motion_sys_config;
    HardwareConfig& hardware_config;
    MotorDriver<SpiDriver> driver = MotorDriver{spi_comms};
    MotionController<SpiDriver, LMSConf> motion_controller =
        MotionController{spi_comms, linear_motion_sys_config, hardware_config};
};

}  // namespace motor_class
