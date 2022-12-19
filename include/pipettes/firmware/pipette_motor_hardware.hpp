#pragma once

#include <atomic>
#include <cstdint>

#include "common/firmware/gpio.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

namespace pipette_motor_hardware {

struct HardwareConfig {
    gpio::PinConfig direction;
    gpio::PinConfig step;
    gpio::PinConfig enable;
    gpio::PinConfig limit_switch;
    gpio::PinConfig led;
    gpio::PinConfig sync_in;
    gpio::PinConfig tip_sense;
    gpio::PinConfig estop_in;
};

class MotorHardware : public motor_hardware::PipetteStepperMotorHardwareIface {
  public:
    ~MotorHardware() final = default;
    MotorHardware() = delete;
    MotorHardware(const HardwareConfig& config, void* timer_handle,
                  void* encoder_handle)
        : pins(config), tim_handle(timer_handle), enc_handle(encoder_handle) {}
    MotorHardware(const MotorHardware&) = delete;
    auto operator=(const MotorHardware&) -> MotorHardware& = delete;
    MotorHardware(MotorHardware&&) = delete;
    auto operator=(MotorHardware&&) -> MotorHardware& = delete;
    void step() final;
    void unstep() final;
    void positive_direction() final;
    void negative_direction() final;
    void activate_motor() final;
    void deactivate_motor() final;
    void start_timer_interrupt() final;
    void stop_timer_interrupt() final;
    auto check_limit_switch() -> bool { return limit.load(); }
    auto check_estop_in() -> bool { return estop.load(); }
    auto check_sync_in() -> bool { return sync.load(); }
    auto check_tip_sense() -> bool { return tip_sense.load(); }
    void read_limit_switch() final;
    void read_estop_in() final;
    void read_sync_in() final;
    void read_tip_sense() final;
    void set_LED(bool status) final;
    auto get_encoder_pulses() -> int32_t final;
    void reset_encoder_pulses() final;
    void encoder_overflow(int32_t direction);

  private:
    std::atomic_bool estop = false;
    std::atomic_bool limit = false;
    std::atomic_bool sync = false;
    std::atomic_bool tip_sense = false;
    HardwareConfig pins;
    void* tim_handle;
    void* enc_handle;
    int32_t encoder_overflow_count = 0;
};

};  // namespace pipette_motor_hardware
