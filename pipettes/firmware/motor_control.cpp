#include "pipettes/firmware/motor_control.hpp"
#include "firmware/common/motor.h"

using namespace motor_control

/*
 * Private Functions
 */
MotorControl::build_command(MotorRegisters command, uint32_t& data, std::span<uint8_t>& output) {
// need to pass in data parameter and use int_to_bytes here
    output = bit_utils::int_to_bytes(command, output);
    output = bit_utils::int_to_bytes(data, output);
}

MotorControl::reset_data() {
    uint32_t clear = 0x0;
    return data & clear;
}

/*
 * Public Functions
 */
MotorControl::set_speed(uint32_t s) {
    auto txBuffer = std::array<uint8_t, 5>{};
    build_command(MotorRegisters.WRITE | MotorRegisters, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
}

MotorControl::get_speed() {
    auto txBuffer = std::array<uint8_t, 5>{};
    build_command(MotorRegisters.WRITE | MotorRegisters, data, txBuffer);
    spi_comms.send_command(txBuffer, data, status);

}

MotorControl::move() {
    Set_Direction();
    Set_Step();
}

MotorControl::setup() {
    // GCONF 0x01
    // IHOLD_IRUN 0x1010
    // CHOPCONF 0x8008
    auto txBuffer = std::array<uint8_t, 5>{};
    uint32_t gconf_data = 0x01;
    uint32_t ihold_irun_data  0x1010;
    uint32_t chopconf = 0x8008;
    build_command(MotorRegisters.WRITE | MotorRegisters.GCONF, gconf_data, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
    build_command(MotorRegisters.WRITE | MotorRegisters.IHOLD_IRUN, ihold_irun_data, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
    build_command(MotorRegisters.WRITE | MotorRegisters.CHOPCONF, chopconf, txBuffer);
    spi_comms.send_command(txBuffer, data, status);

}

MotorControl::get_status() {
    auto txBuffer = std::array<uint8_t, 5>{};
    build_command(MotorRegisters.GSTAT, data, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
}

MotorControl::stop() {
    Reset_Step();
}