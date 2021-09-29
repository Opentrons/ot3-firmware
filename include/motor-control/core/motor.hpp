#pragma once

#include <variant>

#include "common/core/message_queue.hpp"
#include "linear_motion_system.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "motor_messages.hpp"
#include "spi.hpp"

namespace motor_class {

template <spi::TMC2130Spi SpiDriver, template <class> class QueueImpl,
          lms::MotorMechanicalConfig MEConfig>
requires MessageQueue<QueueImpl<Move>, Move>
struct Motor {
    using GenericQueue = QueueImpl<Move>;
    Motor(SpiDriver& spi, lms::LinearMotionSystemConfig<MEConfig> lms_config,
          motion_controller::HardwareConfig& config, GenericQueue& queue)
        : queue(queue),
          driver{spi},
          motion_controller{lms_config, config, queue} {}
    GenericQueue& queue;
    motor_driver::MotorDriver<SpiDriver> driver;
    motion_controller::MotionController<QueueImpl, MEConfig> motion_controller;
    Motor(const Motor&) = delete;
};

}  // namespace motor_class
