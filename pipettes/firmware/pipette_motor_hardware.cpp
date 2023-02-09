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
    // only set the state if the bounce matches the current gpio_is_set
    // on the first state change it won't match but on the second tick it will
    // and we can set it to the new state.
    std::atomic_bool new_state = gpio::is_set(pins.estop_in);
    limit.store(new_state == limit_bounce ? new_state : limit);
    limit_bounce.store(new_state);
}

void MotorHardware::read_estop_in() {
    // only set the state if the bounce matches the current gpio_is_set
    // on the first state change it won't match but on the second tick it will
    // and we can set it to the new state.
    std::atomic_bool new_state = gpio::is_set(pins.estop_in);
    estop.store(new_state == estop_bounce ? new_state : estop);
    estop_bounce.store(new_state);
}

void MotorHardware::read_sync_in() {
    // only set the state if the bounce matches the current gpio_is_set
    // on the first state change it won't match but on the second tick it will
    // and we can set it to the new state.
    std::atomic_bool new_state = gpio::is_set(pins.sync_in);
    sync.store(new_state == sync_bounce ? new_state : sync);
    sync_bounce.store(new_state);
}

void MotorHardware::read_tip_sense() {
    // only set the state if the bounce matches the current gpio_is_set
    // on the first state change it won't match but on the second tick it will
    // and we can set it to the new state.
    std::atomic_bool new_state = gpio::is_set(pins.tip_sense);
    tip_sense.store(new_state == tip_sense_bounce ? new_state : tip_sense);
    tip_sense_bounce.store(new_state);
}

void MotorHardware::set_LED(bool status) {
    if (status) {
        gpio::set(pins.led);
    } else {
        gpio::reset(pins.led);
    }
}

auto MotorHardware::get_encoder_pulses() -> int32_t {
    // Since our overflow count is the high bits of a 32 bit value while
    // the counter is the low 16 bits (see below), we can just bit-pack
    // the value and everything will work.
    if (enc_handle == nullptr) {
        return 0;
    }
    return (encoder_overflow_count << 16) +
           motor_hardware_encoder_pulse_count(enc_handle);
}

void MotorHardware::reset_encoder_pulses() {
    if (enc_handle == nullptr) {
        return;
    }
    motor_hardware_reset_encoder_count(enc_handle);
    encoder_overflow_count = 0;
}
void MotorHardware::encoder_overflow(int32_t direction) {
    // The overflow counter is a signed value that counts the net number
    // of overflows, positive or negative - i.e., if we overflow positive
    // and then positive, this value is 2; positive then negative, 0;
    // etc. That means that it represents a value starting at bit 16 of
    // the 32 bit value of accumulated position, while the encoder count
    // register represents the low 16 bits at any given time.
    encoder_overflow_count += direction;
}
