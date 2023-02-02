#pragma once

#include <atomic>
#include <cstdint>

#include "common/firmware/gpio.hpp"
#include "motor-control/core/types.hpp"

#include "common/core/logging.h"
#include "motor-control/firmware/motor_control_hardware.h"

namespace motor_hardware {

struct HardwareConfig {
    gpio::PinConfig direction;
    gpio::PinConfig step;
    gpio::PinConfig enable;
    gpio::PinConfig limit_switch;
    gpio::PinConfig led;
    gpio::PinConfig sync_in;
    gpio::PinConfig estop_in;
};

struct HardwareConfigForHead {
    gpio::PinConfig direction;
    gpio::PinConfig step;
    gpio::PinConfig enable;
    gpio::PinConfig limit_switch;
    gpio::PinConfig led;
    gpio::PinConfig sync_in;
    gpio::PinConfig estop_in;
    gpio::PinConfig ebrake;
};



template<typename HardwarePinConfigurations>
class MotorHardware {
  public:
    ~MotorHardware() = default;
    MotorHardware() = delete;
    MotorHardware(const HardwarePinConfigurations& config, void* timer_handle,
                  void* encoder_handle)
        : pins(config), tim_handle(timer_handle), enc_handle(encoder_handle) {}
    MotorHardware(const MotorHardware&) = delete;
    auto operator=(const MotorHardware&) -> MotorHardware& = delete;
    MotorHardware(MotorHardware&&) = delete;
    auto operator=(MotorHardware&&) -> MotorHardware& = delete;
  
    void step() { gpio::set(pins.step); }
  
    void unstep() { gpio::reset(pins.step); }

    void positive_direction() { gpio::set(pins.direction); }
    void negative_direction() { gpio::reset(pins.direction); }

    void activate_motor();
    void deactivate_motor();


    void start_timer_interrupt() {
      LOG("Starting timer interrupt")
      motor_hardware_start_timer(tim_handle); 
    }

    void stop_timer_interrupt() { motor_hardware_stop_timer(tim_handle); }
    auto check_limit_switch() -> bool { return limit.load(); }
    auto check_estop_in() -> bool { return estop.load(); }
    auto check_sync_in() -> bool { return sync.load(); }

    [[nodiscard]] auto get_step_tracker() const -> uint32_t { return step_tracker.load();}
    auto reset_step_tracker() -> void { set_step_tracker(0); }
    auto set_step_tracker(uint32_t val) -> void { step_tracker.store(val); }

    void read_limit_switch() { limit = (gpio::is_set(pins.limit_switch)); }
    void read_estop_in() { estop = gpio::is_set(pins.estop_in); }
    void read_sync_in() { sync = (gpio::is_set(pins.sync_in)); }

    void set_LED(bool status) {
      if (status) {
        gpio::set(pins.led);
      } else {
        gpio::reset(pins.led);
    }
    }
    auto get_encoder_pulses() -> int32_t {
      // Since our overflow count is the high bits of a 32 bit value while
      // the counter is the low 16 bits (see below), we can just bit-pack
      // the value and everything will work.
      if (!enc_handle) {
          return 0;
      }
      return (motor_encoder_overflow_count << 16) +
            motor_hardware_encoder_pulse_count(enc_handle);
    }
  
    void reset_encoder_pulses() {
      if (!enc_handle) {
        return;
      }
      motor_hardware_reset_encoder_count(enc_handle);
      motor_encoder_overflow_count = 0;
    }

    // downward interface - call from timer overflow handler
    void encoder_overflow(int32_t direction) {
      // The overflow counter is a signed value that counts the net number
      // of overflows, positive or negative - i.e., if we overflow positive
      // and then positive, this value is 2; positive then negative, 0;
      // etc. That means that it represents a value starting at bit 16 of
      // the 32 bit value of accumulated position, while the encoder count
      // register represents the low 16 bits at any given time.
      motor_encoder_overflow_count += direction;
    }

    MotorPositionStatus position_flags{};

  private:
    std::atomic_bool estop = false;
    std::atomic_bool limit = false;
    std::atomic_bool sync = false;
    HardwarePinConfigurations pins;
    void* tim_handle;
    void* enc_handle;
    int32_t motor_encoder_overflow_count = 0;

    // Used to track the position in microsteps.
    std::atomic<uint32_t> step_tracker{0};
    
};

}  // namespace motor_hardware
