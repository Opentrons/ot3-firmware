#pragma once

#include "hepa-uv/core/light_control_task.hpp"

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
    void set_led_power(uint8_t button, uint8_t led, uint32_t duty_cycle) final;
    void set_button_led_power(uint8_t button, uint32_t r, uint32_t g, uint32_t b, uint32_t w) final;
};

}  // namespace light_control_hardware