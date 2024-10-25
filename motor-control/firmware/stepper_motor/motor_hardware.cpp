#include "motor-control/firmware/stepper_motor/motor_hardware.hpp"

#include <tuple>

#include "common/core/logging.h"
#include "common/firmware/gpio.hpp"
#include "motor-control/firmware/motor_control_hardware.h"

using namespace motor_hardware;

void MotorHardware::step() { gpio::set(pins.step); }

void MotorHardware::unstep() { gpio::reset(pins.step); }

void MotorHardware::positive_direction() { gpio::set(pins.direction); }
void MotorHardware::negative_direction() { gpio::reset(pins.direction); }
void MotorHardware::activate_motor() {
    gpio::set(pins.enable);
    if (pins.ebrake.has_value()) {
        // allow time for the motor current to stablize before releasing the
        // brake
        motor_hardware_delay(20);
        gpio::reset(pins.ebrake.value());
        motor_hardware_delay(20);
    }
}
void MotorHardware::deactivate_motor() {
    if (pins.ebrake.has_value()) {
        gpio::set(pins.ebrake.value());
        motor_hardware_delay(20);
    }
    gpio::reset(pins.enable);
}
void MotorHardware::start_timer_interrupt() {
    LOG("Starting timer interrupt")
    motor_hardware_start_timer(tim_handle);
}
void MotorHardware::stop_timer_interrupt() {
    motor_hardware_stop_timer(tim_handle);
}

bool MotorHardware::is_timer_interrupt_running() {
    return motor_hardware_timer_running(tim_handle);
}

void MotorHardware::read_limit_switch() {
    limit.debounce_update(gpio::is_set(pins.limit_switch));
}

void MotorHardware::read_estop_in() {
    estop.debounce_update(gpio::is_set(pins.estop_in));
}

void MotorHardware::read_sync_in() { sync = gpio::is_set(pins.sync_in); }

void MotorHardware::read_tmc_diag0() {
    diag.debounce_update(gpio::is_set(pins.diag0));
}

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
    if (!enc_handle) {
        return 0;
    }
    int8_t overflows = 0;
    uint16_t pulses = motor_hardware_encoder_pulse_count_with_overflow(
        enc_handle, &overflows);
    motor_encoder_overflow_count += overflows;
    return (motor_encoder_overflow_count << 16) + static_cast<int32_t>(pulses) -
           static_cast<int32_t>(encoder_reset_offset);
}

void MotorHardware::reset_encoder_pulses() {
    if (!enc_handle) {
        return;
    }
    motor_hardware_reset_encoder_count(enc_handle, encoder_reset_offset);
    motor_encoder_overflow_count = 0;
}

void MotorHardware::enable_encoder() {
    LOG("Starting encoder interrupt")
    if (!enc_handle) {
        return;
    }
    if (!motor_hardware_encoder_running(enc_handle)) {
        reset_encoder_pulses();
        motor_hardware_start_encoder(enc_handle);
    }
}
void MotorHardware::disable_encoder() {
    if (!enc_handle) {
        return;
    }
    motor_hardware_stop_encoder(enc_handle);
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
