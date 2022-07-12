#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"

#include "common/core/logging.h"
#include "common/firmware/gpio.hpp"
#include "motor-control/firmware/motor_control_hardware.h"

using namespace motor_hardware;

void MotorHardware::step() { gpio::set(pins.step); }

void MotorHardware::unstep() { gpio::reset(pins.step); }

void MotorHardware::positive_direction() { gpio::set(pins.direction); }
void MotorHardware::negative_direction() { gpio::reset(pins.direction); }
void MotorHardware::activate_motor() { gpio::set(pins.enable); }
void MotorHardware::deactivate_motor() { gpio::reset(pins.enable); }
void MotorHardware::start_timer_interrupt() {
    LOG("Starting timer interrupt")
    motor_hardware_start_timer(tim_handle);
}
void MotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(tim_handle);
}
bool MotorHardware::check_limit_switch() {
    return gpio::is_set(pins.limit_switch);
}

bool MotorHardware::check_sync_in() { return gpio::is_set(pins.sync_in); }

void MotorHardware::set_LED(bool status) {
    if (status) {
        gpio::set(pins.led);
    } else {
        gpio::reset(pins.led);
    }
}

int32_t MotorHardware::get_encoder_pulses() {
    // Since our overflow count is the high bits of a 32 bit value while
    // the counter is the low 16 bits (see below), we can just bit-pack
    // the value and everything will work.
    return (motor_encoder_overflow_count << 16) +
           motor_hardware_encoder_pulse_count(enc_handle);
}

void MotorHardware::reset_encoder_pulses() {
    motor_hardware_reset_encoder_count(enc_handle);
    motor_encoder_overflow_count = 0;
}

void MotorHardware::encoder_overflow(int32_t direction) {
    // The overflow counter is a signed value that counts the net number
    // of overflows, positive or negative - i.e., if we overflow positive
    // and then positive, this value is 2; positive then negative, 0;
    // etc. That means that it represents a value starting at bit 16 of
    // the 32 bit value of accumulated position, while the encoder count
    // register represents the low 16 bits at any given time.
    motor_encoder_overflow_count += direction;
}
