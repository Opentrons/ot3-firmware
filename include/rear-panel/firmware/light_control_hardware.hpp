#pragma once

#include "rear-panel/core/tasks/light_control_task.hpp"

namespace light_control_hardware {

class LightControlHardware : public light_control_task::LightControlInterface {
  public:
    ~LightControlHardware() final = default;
    auto initialize() -> void;
    auto set_led_power(uint8_t id, uint32_t duty_cycle) -> void;
};

}  // namespace light_control_hardware