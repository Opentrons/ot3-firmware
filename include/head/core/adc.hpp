#pragma once

#include <concepts>
#include <cstdint>

namespace adc {

struct MillivoltsReadings {
    uint16_t z_motor;
    uint16_t a_motor;
    uint16_t gripper;
};

struct RawADCReadings {
    uint16_t z_motor;
    uint16_t a_motor;
    uint16_t gripper;
};

constexpr int FULLSCALE_VOLTAGE = 3300;
constexpr int ADC_FULLSCALE_OUTPUT = 4095;

class BaseADC {
  public:
    BaseADC() = default;
    virtual ~BaseADC() = default;
    BaseADC(const BaseADC&) = default;
    auto operator=(const BaseADC&) -> BaseADC& = default;
    BaseADC(BaseADC&&) = default;
    auto operator=(BaseADC&&) -> BaseADC& = default;

    virtual auto get_readings() -> RawADCReadings = 0;
};

}  // namespace adc