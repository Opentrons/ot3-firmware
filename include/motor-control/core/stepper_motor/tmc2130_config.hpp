#pragma once
#include "tmc2130_registers.hpp"

namespace tmc2130 {

// R sense and VSF configuration

struct TMC2130MotorCurrentConfig {
    float r_sense;
    float v_sf;
};

struct TMC2130DriverConfig {
    tmc2130::TMC2130RegisterMap registers;
    TMC2130MotorCurrentConfig current_config;
};

}  // namespace tmc2130