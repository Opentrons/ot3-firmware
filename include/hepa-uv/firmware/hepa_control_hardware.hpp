#pragma once

#include "hepa-uv/core/interfaces.hpp"

namespace hepa_control_hardware {

class HepaControlHardware : public hepa_control::HepaControlInterface {
  public:
    HepaControlHardware() = default;
    HepaControlHardware(const HepaControlHardware&) = delete;
    HepaControlHardware(HepaControlHardware&&) = delete;
    auto operator=(HepaControlHardware&&) -> HepaControlHardware& = delete;
    auto operator=(const HepaControlHardware&) -> HepaControlHardware& = delete;
    ~HepaControlHardware() final = default;

    void set_hepa_fan_speed(uint32_t duty_cycle) final;
    auto get_hepa_fan_rpm() -> uint16_t final;
    void reset_hepa_fan_rpm() final;
    void hepa_fan_rpm_irq(uint16_t rpm) final;
    auto enable_tachometer(bool enable) -> bool final;

  private:
    uint16_t hepa_fan_rpm = 0;
};

}  // namespace hepa_control_hardware
