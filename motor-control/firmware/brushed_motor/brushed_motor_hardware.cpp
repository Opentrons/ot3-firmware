#include "motor-control/firmware/brushed_motor/brushed_motor_hardware.hpp"

#include "common/firmware/gpio.hpp"
#include "motor-control/firmware/motor_control_hardware.h"

using namespace motor_hardware;

void BrushedMotorHardware::positive_direction() {
    if (control_dir != ControlDirection::positive) {
        motor_hardware_stop_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
        motor_hardware_start_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
        control_dir = ControlDirection::positive;
    }
}

void BrushedMotorHardware::negative_direction() {
    if (control_dir != ControlDirection::negative) {
        motor_hardware_start_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
        motor_hardware_stop_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
        control_dir = ControlDirection::negative;
    }
}

void BrushedMotorHardware::start_timer_interrupt() {
    motor_hardware_start_timer(pins.pwm_2.tim);  // start base timer
}
void BrushedMotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(pins.pwm_2.tim);  // stop base timer
}

void BrushedMotorHardware::activate_motor() { gpio::set(pins.enable); }

void BrushedMotorHardware::deactivate_motor() { gpio::reset(pins.enable); }

void BrushedMotorHardware::read_limit_switch() {
    limit = gpio::is_set(pins.limit_switch);
}

void BrushedMotorHardware::read_estop_in() {
    estop = gpio::is_set(pins.estop_in);
}

void BrushedMotorHardware::grip() { positive_direction(); }

void BrushedMotorHardware::ungrip() { negative_direction(); }

void BrushedMotorHardware::stop_pwm() {
    motor_hardware_stop_pwm(pins.pwm_1.tim, pins.pwm_1.channel);
    motor_hardware_stop_pwm(pins.pwm_2.tim, pins.pwm_2.channel);
    control_dir = ControlDirection::unset;
}

void BrushedMotorHardware::read_sync_in() { sync = gpio::is_set(pins.sync_in); }

int32_t BrushedMotorHardware::get_encoder_pulses() {
    if (!enc_handle) {
        return 0;
    }
    return (motor_encoder_overflow_count << 16) +
           motor_hardware_encoder_pulse_count(enc_handle);
}

void BrushedMotorHardware::reset_encoder_pulses() {
    if (!enc_handle) {
        return;
    }
    motor_hardware_reset_encoder_count(enc_handle);
    motor_encoder_overflow_count = 0;
}

void BrushedMotorHardware::encoder_overflow(int32_t direction) {
    motor_encoder_overflow_count += direction;
}

double BrushedMotorHardware::update_control(int32_t encoder_error) {
    return controller_loop.compute(encoder_error);
}

void BrushedMotorHardware::reset_control() { controller_loop.reset(); }
