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
    uint16_t get_hepa_fan_rpm() final;
    void reset_hepa_fan_rpm() final;
    void hepa_fan_rpm_irq(uint16_t rpm) final;
    bool enable_tachometer(bool enable) final;

  private:
    uint16_t hepa_fan_rpm = 0;
};

}  // namespace hepa_control_hardware