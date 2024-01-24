#pragma once

#include "hepa-uv/core/interfaces.hpp"

namespace hepa_control_hardware {

using namespace hepa_control;

class HepaControlHardware : public HepaControlInterface {
  public:
    HepaControlHardware() = default;
    HepaControlHardware(const HepaControlHardware&) = delete;
    HepaControlHardware(HepaControlHardware&&) = delete;
    auto operator=(HepaControlHardware&&) -> HepaControlHardware& = delete;
    auto operator=(const HepaControlHardware&) -> HepaControlHardware& = delete;
    ~HepaControlHardware() final = default;

    void set_hepa_fan_speed(uint32_t duty_cycle);
};

}  // namespace hepa_control_hardware