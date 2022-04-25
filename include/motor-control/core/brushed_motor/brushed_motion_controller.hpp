#pragma once

#include "motor-control/core/linear_motion_system.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

namespace brushed_motion_controller {

using namespace motor_hardware;

class MotionController {
  public:
    using GenericQueue = freertos_message_queue::FreeRTOSMessageQueue<Move>;
    MotionController(BrushedMotorHardwareIface& hardware_iface,
                     GenericQueue& queue)
        : hardware(hardware_iface), queue(queue) {}

    auto operator=(const MotionController&) -> MotionController& = delete;
    auto operator=(MotionController&&) -> MotionController&& = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;
    ~MotionController() = default;

  private:
    BrushedMotorHardwareIface& hardware;
    GenericQueue& queue;
};

}  // namespace brushed_motion_controller