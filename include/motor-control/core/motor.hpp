#pragma once

#include <variant>

#include "common/core/message_queue.hpp"
#include "linear_motion_system.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "motor_driver_config.hpp"
#include "motor_messages.hpp"
#include "spi.hpp"

namespace motor_class {

using namespace motor_messages;
using namespace motor_driver_config;

template <template <class> class PendingQueueImpl,
          template <class> class CompletedQueueImpl,
          lms::MotorMechanicalConfig MEConfig>
requires MessageQueue<PendingQueueImpl<Move>, Move> &&
    MessageQueue<CompletedQueueImpl<Ack>, Ack>
struct Motor {
    using GenericQueue = PendingQueueImpl<Move>;
    using CompletedQueue = CompletedQueueImpl<Ack>;

    /**
     * Construct a motor
     *
     * @param spi SPI driver
     * @param lms_config Linear motion system configuration
     * @param config Pin configuration
     * @param constraints Motion constraints
     * @param driver_config Driver configuration
     * @param queue Input message queue containing motion commands.
     * @param completed_queue Output message queue with completed motion
     * commands.
     */
    Motor(spi::TMC2130Spi& spi,
          lms::LinearMotionSystemConfig<MEConfig> lms_config,
          motion_controller::HardwareConfig config,
          MotionConstraints constraints, RegisterConfig driver_config,
          GenericQueue& queue, CompletedQueue& completed_queue)
        : pending_move_queue(queue),
          completed_move_queue(completed_queue),
          driver{spi, driver_config},
          motion_controller{lms_config, config, constraints, pending_move_queue,
                            completed_move_queue} {}
    GenericQueue& pending_move_queue;
    CompletedQueue& completed_move_queue;
    motor_driver::MotorDriver driver;
    motion_controller::MotionController<PendingQueueImpl, CompletedQueueImpl,
                                        MEConfig>
        motion_controller;
    Motor(const Motor&) = delete;
    auto operator=(const Motor&) -> Motor& = delete;
    Motor(Motor&&) = delete;
    auto operator=(Motor&&) -> Motor&& = delete;
    ~Motor() = default;
};

}  // namespace motor_class
