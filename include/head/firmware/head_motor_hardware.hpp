#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"

namespace head_motor_hardware {

struct HeadHardwareConfig : public motor_hardware::HardwareConfig {
	gpio::PinConfig ebrake;
};

} // head_motor_hardware
