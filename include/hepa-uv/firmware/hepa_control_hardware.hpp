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
    uint32_t get_hepa_fan_rpm() final;
};

}  // namespace hepa_control_hardware