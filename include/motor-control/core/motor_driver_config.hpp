#pragma once

#include <map>

namespace motor_driver_config {

enum class DriverRegisters : uint8_t {
    GCONF = 0x00,
    GSTAT = 0x01,
    IOIN = 0x04,
    IHOLD_IRUN = 0x10,
    TPOWERDOWN = 0x11,
    TPWMTHRS = 0x13,
    TCOOLTHRS = 0x14,
    THIGH = 0x15,
    XDIRECT = 0x2D,
    VDCMIN = 0x33,
    CHOPCONF = 0x6C,
    COOLCONF = 0x6D,
    DCCTRL = 0x6E,
    DRVSTATUS = 0x6F,
    PWMCONF = 0x70,
    ENCM_CTRL = 0x72
};

typedef std::map<DriverRegisters, uint32_t> RegisterConfig;

}  // namespace motor_driver_config
