#pragma once

#include <atomic>
#include <cstdint>
#include <optional>

#include "common/core/debounce.hpp"
#include "common/firmware/gpio.hpp"
#include "motor-control/core/motor_hardware_interface.hpp"

namespace motor_hardware {

struct HardwareConfig {
    gpio::PinConfig direction;
    gpio::PinConfig step;
    gpio::PinConfig enable;
    gpio::PinConfig limit_switch;
    gpio::PinConfig led;
    gpio::PinConfig sync_in;
    gpio::PinConfig estop_in;
    std::optional<gpio::PinConfig> ebrake = std::nullopt;
};

class MotorHardware : public StepperMotorHardwareIface {
  public:
    ~MotorHardware() final = default;
    MotorHardware() = delete;
    MotorHardware(const HardwareConfig& config, void* timer_handle,
                  void* encoder_handle, const UsageEEpromConfig& eeprom_config)
        : pins(config),
          tim_handle(timer_handle),
          enc_handle(encoder_handle),
          eeprom_config(eeprom_config) {}
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
    auto check_limit_switch() -> bool final { return limit.debounce_state(); }
    auto check_estop_in() -> bool final { return estop.debounce_state(); }
    auto check_sync_in() -> bool final { return sync.debounce_state(); }
    void read_limit_switch() final;
    void read_estop_in() final;
    void read_sync_in() final;
    void set_LED(bool status) final;
    auto get_encoder_pulses() -> int32_t final;
    void reset_encoder_pulses() final;
    auto get_usage_eeprom_config() -> UsageEEpromConfig& {
        return eeprom_config;
    }
    // downward interface - call from timer overflow handler
    void encoder_overflow(int32_t direction);

  private:
    debouncer::Debouncer estop = debouncer::Debouncer{};
    debouncer::Debouncer limit = debouncer::Debouncer{};
    debouncer::Debouncer sync = debouncer::Debouncer{};
    HardwareConfig pins;
    void* tim_handle;
    void* enc_handle;
    UsageEEpromConfig eeprom_config;
    int32_t motor_encoder_overflow_count = 0;
};

};  // namespace motor_hardware
