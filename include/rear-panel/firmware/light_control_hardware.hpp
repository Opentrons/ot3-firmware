#pragma once

#include "rear-panel/core/tasks/light_control_task.hpp"

namespace light_control_hardware {

class LightControlHardware : public light_control_task::LightControlInterface {
  public:
    LightControlHardware() = default;
    LightControlHardware(const LightControlHardware&) = delete;
    LightControlHardware(LightControlHardware&&) = delete;
    auto operator=(LightControlHardware&&) -> LightControlHardware& = delete;
    auto operator=(const LightControlHardware&)
        -> LightControlHardware& = delete;
    ~LightControlHardware() final = default;

    auto initialize() -> void;
    void set_led_power(uint8_t id, uint32_t duty_cycle) final;
};

}  // namespace light_control_hardware