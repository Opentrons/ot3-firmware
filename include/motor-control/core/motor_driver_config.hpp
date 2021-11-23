#pragma once

#include <array>

namespace motor_driver_config {

struct DriverRegisters {
    enum Addresses : uint8_t {
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
        ENCM_CTRL = 0x72,
        TSTEP = 0x12,
        MSLUT_0 = 0x60,
        MSLUT_1 = 0x61,
        MSLUT_2 = 0x62,
        MSLUT_3 = 0x63,
        MSLUT_4 = 0x64,
        MSLUT_5 = 0x65,
        MSLUT_6 = 0x66,
        MSLUT_7 = 0x67,
        MSLUTSEL = 0x68,
        MSLUTSTART = 0x69,
        MSCNT = 0x6A,
        MSCURACT = 0x6B,
        PWM_SCALE = 0x71,
        LOST_STEPS = 0x73,
    };

    static auto is_valid_address(const uint8_t add) -> bool {
        switch (static_cast<Addresses>(add)) {
            case GCONF:
            case GSTAT:
            case IOIN:
            case IHOLD_IRUN:
            case TPOWERDOWN:
            case TPWMTHRS:
            case TCOOLTHRS:
            case THIGH:
            case XDIRECT:
            case VDCMIN:
            case CHOPCONF:
            case COOLCONF:
            case DCCTRL:
            case DRVSTATUS:
            case PWMCONF:
            case ENCM_CTRL:
            case TSTEP:
            case MSLUT_0:
            case MSLUT_1:
            case MSLUT_2:
            case MSLUT_3:
            case MSLUT_4:
            case MSLUT_5:
            case MSLUT_6:
            case MSLUT_7:
            case MSLUTSEL:
            case MSLUTSTART:
            case MSCNT:
            case MSCURACT:
            case PWM_SCALE:
            case LOST_STEPS:
                return true;
        }
        return false;
    }
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

}  // namespace motor_driver_config
