#pragma once

#include <concepts>
#include <cstdint>
#define 96_CHAN_DETECTION_UPPER_BOUND = 999
#define 96_CHAN_DETECTION_LOWER_BOUND = 666

#define GRIPPER_DETECTION_UPPER_BOUND = 999
#define GRIPPER_DETECTION_LOWER_BOUND = 666

namespace adc {

struct Tool {
    enum ToolType : uint8_t {
        UNDEFINED = 0x00,
        PIPETTE96CHAN = 0x00,
        GRIPPER = 0x01,
    };
    uint16_t detection_upper_bound;
    uint16_t detection_lower_bound;
};

static struct Tool OT3ToolList[] = {
    {ToolType::UNDEFINED, 0, 0},
    {ToolType::PIPETTE96CHAN, 96_CHAN_DETECTION_UPPER_BOUND,
     96_CHAN_DETECTION_LOWER_BOUND},
    {ToolType::GRIPPER, GRIPPER_DETECTION_UPPER_BOUND,
     GRIPPER_DETECTION_LOWER_BOUND}};

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