#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"

#include "common/core/logging.h"
#include "common/firmware/gpio.hpp"
#include "motor-control/firmware/motor_control_hardware.h"

namespace motor_hardware {

template <>
void MotorHardware<HardwareConfig>::activate_motor() {
    gpio::set(pins.enable);
}

template <>
void MotorHardware<HardwareConfig>::deactivate_motor() {
    gpio::reset(pins.enable);
}

template <>
void MotorHardware<HardwareConfigForHead>::activate_motor() {
    gpio::reset(pins.ebrake);
    gpio::set(pins.enable);
}

template <>
void MotorHardware<HardwareConfigForHead>::deactivate_motor() {
    gpio::set(pins.ebrake);
    gpio::reset(pins.enable);
}

}  // namespace motor_hardware
