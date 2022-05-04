#pragma once

#include "common/core/freertos_message_queue.hpp"
#include "driver_interface.hpp"
#include "motor-control/core/brushed_motor/brushed_motion_controller.hpp"
#include "motor-control/core/motor_messages.hpp"

namespace motor_hardware {
class MotorHardwareIface;
};

namespace brushed_motor {

using namespace motor_messages;
using namespace freertos_message_queue;

struct BrushedMotor {
    using GenericQueue = FreeRTOSMessageQueue<BrushedMove>;

  public:
    BrushedMotor(motor_hardware::BrushedMotorHardwareIface& hardware_iface,
                 brushed_motor_driver::BrushedMotorDriverIface& driver_iface,
                 GenericQueue& queue)
        : driver(driver_iface), motion_controller{hardware_iface, queue} {}

    brushed_motor_driver::BrushedMotorDriverIface& driver;
    brushed_motion_controller::MotionController motion_controller;

    BrushedMotor(const BrushedMotor&) = delete;
    auto operator=(const BrushedMotor&) -> BrushedMotor& = delete;
    BrushedMotor(BrushedMotor&&) = delete;
    auto operator=(BrushedMotor&&) -> BrushedMotor&& = delete;
    ~BrushedMotor() = default;
};

}  // namespace brushed_motor