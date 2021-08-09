#pragma once

#include <type_traits>

#include "common/firmware/motor.h"
#include "spi.hpp"

namespace motion_controller {

template <typename SpiDriver>
requires spi::TMC2130Spi<SpiDriver, spi::BufferSize>
class MotionController {
  public:
    MotionController(SpiDriver& spi) : spi_comms(spi) {}

    void set_speed(uint32_t s) { speed = s; }

    void set_direction(bool d) {
        if (d) {
            Set_Direction();
        } else {
            Reset_Direction();
        }
        direction = d;
    }

    void set_acceleration(uint32_t a) { acc = a; }
    void set_distance();

    void move() {
        Set_Enable_Pin();
        Set_Direction();
        const int tries = 10000;
        for (int i = 0; i < tries; i++) {
            Set_Step();
            Reset_Step();
        }
    }

    void stop() {
        Reset_Step();
        Reset_Enable_Pin();
    }

    auto get_speed() -> uint32_t { return speed; }
    auto get_acceleration() -> uint32_t { return acc; }
    auto get_direction() -> bool { return direction; }

  private:
    uint32_t acc = 0x0;
    uint32_t speed = 0x0;
    bool direction = true;  // direction true: forward, false: backward
    SpiDriver spi_comms;
};

}  // namespace motion_controller
