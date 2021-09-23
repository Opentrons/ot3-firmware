#pragma once

#include <type_traits>

#include "common/firmware/motor.h"
#include "common/firmware/timer_interrupt.h"
#include "motor_messages.hpp"
#include "spi.hpp"
#include "step_motor.hpp"

using namespace motor_messages;

namespace motion_controller {

struct HardwareConfig {
    struct PinConfig direction;
    struct PinConfig step;
    struct PinConfig enable;
};

template <spi::TMC2130Spi SpiDriver, template <class> class QueueImpl>
requires MessageQueue<QueueImpl<Move>, Move>
class MotionController {
  public:
    using GenericQueue = QueueImpl<Move>;
    explicit MotionController(SpiDriver& spi, HardwareConfig& config,
                              GenericQueue& queue)
        : spi_comms(spi), hardware_config(config), queue(queue) {
        timer_init();
        setup();
    }

    void setup() {
        start_motor_handler(queue);
        timer_interrupt_start();
    }

    void set_speed(uint32_t s) { speed = s; }

    void set_direction(bool d) {
        if (d) {
            set_pin(hardware_config.direction);
        } else {
            reset_pin(hardware_config.direction);
        }
        direction = d;
    }

    void set_acceleration(uint32_t a) { acc = a; }
    void set_distance(uint32_t d) { dist = d; }

    void move(const Move& msg) {
        set_pin(hardware_config.enable);
        set_pin(hardware_config.direction);
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
    uint32_t speed = 0x0;
    uint32_t dist = 0x0;
    bool direction = true;  // direction true: forward, false: backward
    SpiDriver& spi_comms;
    HardwareConfig& hardware_config;
    GenericQueue& queue;
};

}  // namespace motion_controller
