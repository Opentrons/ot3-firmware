#pragma once

#include <variant>

#include "common/core/freertos_message_queue.hpp"
#include "linear_motion_system.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "motor_driver_config.hpp"
#include "motor_messages.hpp"
#include "spi.hpp"

namespace motor_hardware {
class MotorHardwareIface;
};

namespace motor_class {

using namespace motor_messages;
using namespace motor_driver_config;
using namespace freertos_message_queue;

template <spi::TMC2130Spi SpiDriver, lms::MotorMechanicalConfig MEConfig>
struct Motor {
    using GenericQueue = FreeRTOSMessageQueue<motor_messages::Move>;
    using CompletedQueue = FreeRTOSMessageQueue<motor_messages::Ack>;
    Motor(SpiDriver& spi, lms::LinearMotionSystemConfig<MEConfig> lms_config,
          motor_hardware::MotorHardwareIface& hardware_iface,
          MotionConstraints constraints, RegisterConfig driver_config,
          GenericQueue& queue, CompletedQueue& completed_queue)
        : pending_move_queue(queue),
          completed_move_queue(completed_queue),
          driver{spi, driver_config},
          motion_controller{lms_config, hardware_iface, constraints,
                            pending_move_queue, completed_move_queue} {}
    GenericQueue& pending_move_queue;
    CompletedQueue& completed_move_queue;
    motor_driver::MotorDriver<SpiDriver> driver;
    motion_controller::MotionController<MEConfig> motion_controller;
    Motor(const Motor&) = delete;
    auto operator=(const Motor&) -> Motor& = delete;
    Motor(Motor&&) = delete;
    auto operator=(Motor&&) -> Motor&& = delete;
    ~Motor() = default;
};

}  // namespace motor_class
