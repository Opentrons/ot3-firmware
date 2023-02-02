#pragma once

#include <atomic>
#include <cstdint>

#include "common/firmware/gpio.hpp"
#include "motor-control/core/types.hpp"
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

class MotorHardware {
  public:
    ~MotorHardware() = default;
    MotorHardware() = delete;
    MotorHardware(const HardwareConfig& config, void* timer_handle,
                  void* encoder_handle)
        : pins(config), tim_handle(timer_handle), enc_handle(encoder_handle) {}
    MotorHardware(const MotorHardware&) = delete;
    auto operator=(const MotorHardware&) -> MotorHardware& = delete;
    MotorHardware(MotorHardware&&) = delete;
    auto operator=(MotorHardware&&) -> MotorHardware& = delete;
    void step();
    void unstep();
    void positive_direction();
    void negative_direction();
    void activate_motor();
    void deactivate_motor();
    void start_timer_interrupt();
    void stop_timer_interrupt();
    auto check_limit_switch() -> bool { return limit.load(); }
    auto check_estop_in() -> bool { return estop.load(); }
    auto check_sync_in() -> bool { return sync.load(); }
    auto check_tip_sense() -> bool { return tip_sense.load(); }
    void read_limit_switch();
    void read_estop_in();
    void read_sync_in();
    void read_tip_sense();
    void set_LED(bool status);
    auto get_encoder_pulses() -> int32_t;
    void reset_encoder_pulses();
    void encoder_overflow(int32_t direction);

    [[nodiscard]] auto get_step_tracker() const -> uint32_t { return step_tracker.load();}
    auto reset_step_tracker() -> void { set_step_tracker(0); }
    auto set_step_tracker(uint32_t val) -> void { step_tracker.store(val); }

    MotorPositionStatus position_flags{};

  private:
    std::atomic_bool estop = false;
    std::atomic_bool limit = false;
    std::atomic_bool sync = false;
    std::atomic_bool tip_sense = false;
    HardwareConfig pins;
    void* tim_handle;
    void* enc_handle;
    int32_t encoder_overflow_count = 0;

    // Used to track the position in microsteps.
    std::atomic<uint32_t> step_tracker{0};
};

};  // namespace pipette_motor_hardware
