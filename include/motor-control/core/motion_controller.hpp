#pragma once

#include <type_traits>

#include "common/firmware/motor.h"
#include "common/firmware/timer_interrupt.h"
#include "linear_motion_system.hpp"
#include "spi.hpp"

namespace motion_controller {

struct HardwareConfig {
    struct PinConfig direction;
    struct PinConfig step;
    struct PinConfig enable;
};

/*
 * MotionController is responsible for motor movement and communicate with the
 * motor driver using the HAL driver API and SPI.
 */
template <spi::TMC2130Spi SpiDriver, lms::LMSConfig LMSConf>
class MotionController {
  public:
    explicit MotionController(SpiDriver& spi, LMSConf& lms_config,
                              HardwareConfig& config)
        : spi_comms(spi),
          linear_motion_sys_config(lms_config),
          hardware_config(config) {
        timer_init();
        steps_per_mm = linear_motion_sys_config.get_steps_per_mm();
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
    void set_distance(float dist) {
        float total_steps = dist * steps_per_mm;
        // pass total steps to IRS
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
    uint32_t speed = 0x0;   // mm/s
    bool direction = true;  // direction true: forward, false: backward
    float steps_per_mm;
    SpiDriver& spi_comms;
    LMSConf& linear_motion_sys_config;
    HardwareConfig& hardware_config;
};

}  // namespace motion_controller
