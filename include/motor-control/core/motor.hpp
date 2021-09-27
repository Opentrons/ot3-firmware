#pragma once

#include <variant>

#include "common/core/message_queue.hpp"
#include "linear_motion_system.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "motor_messages.hpp"
#include "spi.hpp"

using namespace motor_driver;
using namespace motion_controller;
using namespace lms;

namespace motor_class {

using namespace spi;
using namespace motor_messages;

namespace motor_class {

template <TMC2130Spi SpiDriver, template <class> class QueueImpl,
          lms::LMSConfig LMSConf>
requires MessageQueue<QueueImpl<Move>, Move>
struct Motor {
    using GenericQueue = QueueImpl<Move>;
    explicit Motor(SpiDriver& spi, LMSConf& lms_config, HardwareConfig& config,
                   GenericQueue& queue)
        : spi_comms(spi),
          linear_motion_sys_config(lms_config),
          hardware_config(config),
          queue(queue) {}
    SpiDriver& spi_comms;
    LMSConf& linear_motion_sys_config;
    HardwareConfig& hardware_config;
    GenericQueue& queue;
    MotorDriver<SpiDriver> driver = MotorDriver{spi_comms};
    MotionController<SpiDriver, QueueImpl, LMSConf> motion_controller =
        MotionController{spi_comms, linear_motion_sys_config, hardware_config,
                         queue};
};

}  // namespace motor_class
