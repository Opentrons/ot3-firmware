#include "pipettes/firmware/pipette_motor_hardware.hpp"

#include "common/firmware/gpio.hpp"
#include "motor-control/firmware/motor_control_hardware.h"

using namespace pipette_motor_hardware;

void MotorHardware::step() { gpio::set(pins.step); }

void MotorHardware::unstep() { gpio::reset(pins.step); }

void MotorHardware::positive_direction() { gpio::set(pins.direction); }
void MotorHardware::negative_direction() { gpio::reset(pins.direction); }
void MotorHardware::activate_motor() { gpio::set(pins.enable); }
void MotorHardware::deactivate_motor() { gpio::reset(pins.enable); }
void MotorHardware::start_timer_interrupt() {
    motor_hardware_start_timer(tim_handle);
}
void MotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(tim_handle);
}
void MotorHardware::read_limit_switch() {
    limit.debounce_update(gpio::is_set(pins.limit_switch));
}

void MotorHardware::read_estop_in() {
    estop.debounce_update(gpio::is_set(pins.estop_in));
}

void MotorHardware::read_sync_in() {
    sync.debounce_update(gpio::is_set(pins.sync_in));
}
void MotorHardware::read_tip_sense() {
    tip_sense.debounce_update(gpio::is_set(pins.tip_sense));
}

void MotorHardware::set_LED(bool status) {
    if (status) {
        gpio::set(pins.led);
    } else {
        gpio::reset(pins.led);
    }
}

auto MotorHardware::get_encoder_pulses() -> int32_t { return 0; }

void MotorHardware::reset_encoder_pulses() {}
