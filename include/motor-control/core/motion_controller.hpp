#pragma once

#include <type_traits>

#include "common/firmware/motor.h"
#include "common/firmware/timer_interrupt.h"
#include "linear_motion_system.hpp"
#include "motor_messages.hpp"
#include "step_motor.hpp"

using namespace motor_messages;

namespace motion_controller {

struct HardwareConfig {
    struct PinConfig direction;
    struct PinConfig step;
    struct PinConfig enable;
};
/*
 * MotionController is responsible for motor movement and communicate with
 * the motor driver using the HAL driver API and SPI.
 */
template <template <class> class QueueImpl, lms::MotorMechanicalConfig MEConfig>
requires MessageQueue<QueueImpl<Move>, Move>
class MotionController {
  public:
    using GenericQueue = QueueImpl<Move>;
    MotionController(lms::LinearMotionSystemConfig<MEConfig> lms_config,
                     HardwareConfig& config, GenericQueue& queue)
        : linear_motion_sys_config(lms_config),
          hardware_config(config),
          queue(queue) {
        timer_init();
        setup();
    }

    MotionController& operator=(MotionController&) = delete;
    MotionController&& operator=(MotionController&&) = delete;
    MotionController(MotionController&) = delete;
    MotionController(MotionController&&) = delete;

    ~MotionController() {
        reset_pin(hardware_config.enable);
        timer_interrupt_stop();
    }

    void setup() {
        start_motor_handler(&queue);
        timer_interrupt_start();
        // convert steps to mm to fixed point here
        steps_per_mm =
            static_cast<uint32_t>(linear_motion_sys_config.get_steps_per_mm());
    }

    void set_velocity(sq0_31 v) { velocity = v; }

    void set_acceleration(sq0_31 a) { acceleration = a; }

    void move(const CanMove& can_msg) {
        // TODO: set direction in
        //        motor interrupt handler instead
        uint64_t converted_steps =
            static_cast<int64_t>(can_msg.target_position * steps_per_mm) << 31;
        Move msg{.target_position = converted_steps, .velocity = velocity};
        queue.try_write(msg);
    }

    void enable_motor() { set_pin(hardware_config.enable); }

    void disable_motor() { reset_pin(hardware_config.enable); }

    void stop() {
        reset_pin(hardware_config.step);
        timer_interrupt_stop();
    }

    auto get_speed() -> uint32_t { return speed; }
    auto get_acceleration() -> uint32_t { return acc; }
    auto get_direction() -> bool { return direction; }

  private:
    const sq0_31 default_velocity = 0x1 << 30;
    sq0_31 acceleration = 0x0;
    sq0_31 velocity = 0x0;
    bool direction = true;  // direction true: forward, false: backward
    uint32_t steps_per_mm;
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    HardwareConfig& hardware_config;
    GenericQueue& queue;
};

}  // namespace motion_controller
