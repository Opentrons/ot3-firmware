#pragma once

#include <stdint.h>

#include <concepts>

namespace adc {

struct VoltageRead {
    uint16_t z_motor;
    uint16_t a_motor;
    uint16_t gripper;
};

class BaseADC {
  public:
    BaseADC() = default;
    virtual ~BaseADC() = default;
    BaseADC(const BaseADC&) = default;
    auto operator=(const BaseADC&) -> BaseADC& = default;
    BaseADC(BaseADC&&) = default;
    auto operator=(BaseADC&&) -> BaseADC& = default;

    virtual auto get_readings() -> VoltageRead = 0;
};

}  // namespace adc