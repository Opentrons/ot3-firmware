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

    void setup() {
        start_motor_handler(queue);
        timer_interrupt_start();
        steps_per_mm = linear_motion_sys_config.get_steps_per_mm();
        //        speed = clk_frequency / 2 / steps_per_mm;
        inc_multiplier = 1;
    }

    void set_speed(uint32_t s) {
        speed = s;
        //        inc_multiplier = speed * steps_per_mm / (2 * clk_frequency);
    }

    void set_direction(bool d) {
        if (d) {
            set_pin(hardware_config.direction);
        } else {
            reset_pin(hardware_config.direction);
        }
        direction = d;
    }

    void set_acceleration(uint32_t a) { acc = a; }
    void set_distance(float dist) {
        float total_steps = dist * steps_per_mm;
        // pass total steps to IRS
    }

    void move(const Move& msg) {
        set_pin(hardware_config.enable);
        //        set_pin(hardware_config.direction); // TODO: set direction in
        //        motor interrupt handler instead
        queue.try_write(msg);
    }

    void stop() {
        reset_pin(hardware_config.step);
        reset_pin(hardware_config.enable);
        timer_interrupt_stop();
    }

    auto get_speed() -> uint32_t { return speed; }
    auto get_acceleration() -> uint32_t { return acc; }
    auto get_direction() -> bool { return direction; }

  private:
    uint32_t acc = 0x0;
    uint32_t speed = 0x0;  // mm/s
    uint32_t dist = 0x0;
    uint32_t inc_multiplier = 0x0;
    bool direction = true;  // direction true: forward, false: backward
    float steps_per_mm;
    lms::LinearMotionSystemConfig<MEConfig> linear_motion_sys_config;
    HardwareConfig& hardware_config;
    GenericQueue& queue;
};

}  // namespace motion_controller
