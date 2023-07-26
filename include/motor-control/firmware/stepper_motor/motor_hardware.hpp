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
    auto is_timer_interrupt_running() -> bool final;
    auto check_limit_switch() -> bool final { return limit.debounce_state(); }
    auto check_estop_in() -> bool final { return estop.debounce_state(); }
    auto check_sync_in() -> bool final { return sync; }
    void read_limit_switch() final;
    void read_estop_in() final;
    void read_sync_in() final;
    void set_LED(bool status) final;
    auto get_encoder_pulses() -> int32_t final;
    void reset_encoder_pulses() final;
    auto has_cancel_request() -> bool final {
        return cancel_request.exchange(false);
    }
    void disable_encoder() final;
    void enable_encoder() final;

    void request_cancel() final { cancel_request.store(true); }

    auto get_usage_eeprom_config() -> const UsageEEpromConfig& final {
        return eeprom_config;
    }
    // downward interface - call from timer overflow handler
    void encoder_overflow(int32_t direction);

  private:
    debouncer::Debouncer estop = debouncer::Debouncer{};
    debouncer::Debouncer limit = debouncer::Debouncer{};
    std::atomic_bool sync = false;
    static constexpr uint16_t encoder_reset_offset = 20;
    HardwareConfig pins;
    void* tim_handle;
    void* enc_handle;
    const UsageEEpromConfig& eeprom_config;
    std::atomic<int32_t> motor_encoder_overflow_count = 0;
    std::atomic<bool> cancel_request = false;
    static constexpr uint32_t ENCODER_OVERFLOW_PULSES_BIT = 0x1 << 31;
};

};  // namespace motor_hardware
