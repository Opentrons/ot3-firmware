#pragma once

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

struct RegisterConfig {
    uint32_t gconf = 0;
    uint32_t gstat = 0;
    uint32_t ioin = 0;
    uint32_t ihold_irun = 0;
    uint32_t tpowerdown = 0;
    uint32_t tpwnthrs = 0;
    uint32_t tcoolthrs = 0;
    uint32_t chopconf = 0;
    uint32_t thigh = 0;
    uint32_t coolconf = 0;
    uint32_t xdirect = 0;
    uint32_t vdcmin = 0;
    uint32_t dcctrl = 0;
    uint32_t pwmconf = 0;
    uint32_t encm_ctrl = 0;
};

template <typename RA>
concept RegisterAddress = requires {
    std::is_same_v<RA, DriverRegisters> || std::is_same_v<RA, uint8_t>;
};

}  // namespace motor_driver_config
