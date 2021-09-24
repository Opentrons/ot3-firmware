#pragma once

#include <variant>

#include "common/core/message_queue.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "motor_messages.hpp"
#include "spi.hpp"

using namespace motor_driver;
using namespace motion_controller;
using namespace spi;
using namespace motor_messages;

namespace motor_class {

template <TMC2130Spi SpiDriver, template <class> class QueueImpl>
requires MessageQueue<QueueImpl<Move>, Move>
struct Motor {
    using GenericQueue = QueueImpl<Move>;
    explicit Motor(SpiDriver& spi, HardwareConfig& config, GenericQueue& queue)
        : spi_comms(spi), hardware_config(config), queue(queue) {}
    SpiDriver& spi_comms;
    HardwareConfig& hardware_config;
    GenericQueue& queue;
    MotorDriver<SpiDriver> driver = MotorDriver{spi_comms};
    MotionController<SpiDriver, QueueImpl> motion_controller =
        MotionController{spi_comms, hardware_config, queue};
};

}  // namespace motor_class
