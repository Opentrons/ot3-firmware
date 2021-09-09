#pragma once

#include <type_traits>

#include "common/firmware/motor.h"
#include "common/firmware/timer_interrupt.h"
#include "spi.hpp"

namespace motion_controller {

struct HardwareConfig {
    struct PinConfig direction;
    struct PinConfig step;
    struct PinConfig enable;
};

template <spi::TMC2130Spi SpiDriver>
class MotionController {
  public:
    explicit MotionController(SpiDriver& spi, HardwareConfig& config)
        : spi_comms(spi), hardware_config(config) {
        timer_init();
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
    void set_distance(uint32_t d) {
        dist = d;
        set_steps(d);
    }

    void move() {
        set_pin(hardware_config.enable);
        set_pin(hardware_config.direction);
        timer_interrupt_start();
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
};

}  // namespace motion_controller
