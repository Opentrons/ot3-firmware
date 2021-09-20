#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "motor_interrupt_handler.hpp"
#include "spi.hpp"

using namespace motor_driver;
using namespace motion_controller;
using namespace freertos_message_queue;
using namespace spi;

namespace motor_class {

template <TMC2130Spi SpiDriver, FreeRTOSGenericQueue GenericQueue>
struct Motor {
    explicit Motor(SpiDriver& spi, HardwareConfig& config, GenericQueue isr_queue)
        : spi_comms(spi), hardware_config(config) {}
    SpiDriver& spi_comms;
    HardwareConfig& hardware_config;
    MotorDriver<SpiDriver> driver = MotorDriver{spi_comms};
    MotionController<SpiDriver> motion_controller =
        MotionController{spi_comms, hardware_config};
    MotorInterruptHandler<GenericQueue> interrupt_handler = MotorInterruptHandler{isr_queue};
};

}  // namespace motor_class
