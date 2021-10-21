#pragma once

#include <variant>

#include "common/core/message_queue.hpp"
#include "linear_motion_system.hpp"
#include "motion_controller.hpp"
#include "motor_driver.hpp"
#include "motor_messages.hpp"
#include "spi.hpp"

namespace motor_class {

template <spi::TMC2130Spi SpiDriver, template <class> class PendingQueueImpl, template <class> class CompletedQueueImpl,
          lms::MotorMechanicalConfig MEConfig>
requires MessageQueue<PendingQueueImpl<Move>, Move> && MessageQueue<CompletedQueueImpl<Ack>, Ack>
struct Motor {
    using GenericQueue = PendingQueueImpl<Move>;
    using CompletedQueue = CompletedQueueImpl<Ack>;
    Motor(SpiDriver& spi, lms::LinearMotionSystemConfig<MEConfig> lms_config,
          motion_controller::HardwareConfig& config, GenericQueue& queue, CompletedQueue& completed_queue)
          : pending_move_queue(queue), completed_move_queue(completed_queue),
          driver{spi},
          motion_controller{lms_config, config, pending_move_queue, completed_move_queue} {}
    GenericQueue& pending_move_queue;
    CompletedQueue& completed_move_queue;
    motor_driver::MotorDriver<SpiDriver> driver;
    motion_controller::MotionController<PendingQueueImpl, CompletedQueueImpl, MEConfig> motion_controller;
    Motor(const Motor&) = delete;
};

}  // namespace motor_class
