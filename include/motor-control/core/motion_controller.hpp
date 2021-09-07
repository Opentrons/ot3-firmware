#pragma once

#include <type_traits>

#include "common/firmware/motor.h"
#include "spi.hpp"

namespace motion_controller {

struct HardwareConfig {
    struct PinConfig direction;
    struct PinConfig step;
    struct PinConfig enable;
};

template <typename SpiDriver>
requires spi::TMC2130Spi<SpiDriver>
class MotionController {
  public:
    explicit MotionController(SpiDriver& spi, HardwareConfig& config)
        : spi_comms(spi), hardware_config(config) {}

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
    void set_distance();

    void move() {
        set_pin(hardware_config.enable);
        set_pin(hardware_config.direction);
        start_it();
    }

    void stop() {
        reset_pin(hardware_config.step);
        reset_pin(hardware_config.enable);
    }

    auto get_speed() -> uint32_t { return speed; }
    auto get_acceleration() -> uint32_t { return acc; }
    auto get_direction() -> bool { return direction; }

  private:
    uint32_t acc = 0x0;
    uint32_t speed = 0x0;
    bool direction = true;  // direction true: forward, false: backward
    SpiDriver& spi_comms;
    HardwareConfig& hardware_config;
};

}  // namespace motion_controller
