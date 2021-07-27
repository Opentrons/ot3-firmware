#include "pipettes/firmware/motor_control.hpp"

#include "FreeRTOS.h"
#include "common/core/bit_utils.hpp"
#include "common/firmware/motor.h"
#include "task.h"

using namespace motor_control;

/*
 * Motor Control and namespace.
 * Private:
 * build_command takes in a command address, a reference
 * to the data to be transmitted and the output array to
 * store this in.
 * reset_data takes nothing and resets the global data variable
 *
 * Public Globals:
 * data, the data returned from the driver
 * status, the status of the driver
 *
 * Public:
 * setup, sets the initial configurations of the driver
 * set_speed, sets a global variable called speed
 * get_speed, gets a global variable called speed
 * move, moves the motor for an unlimited amount of time
 * get_status, get the status of the GSTAT register
 */

/*
 * Private Functions
 */
auto MotorControl::build_command(uint8_t command, const uint32_t& command_data)
    -> BufferType {
    // need to pass in data parameter and use int_to_bytes here
    auto txBuffer = BufferType{0};
    auto iter = txBuffer.begin();
    iter = bit_utils::int_to_bytes(command, iter, txBuffer.end());
    // NOLINTNEXTLINE(clang-diagnostic-unused-result)
    bit_utils::int_to_bytes(command_data, iter, txBuffer.end());
    return txBuffer;
}

void MotorControl::reset_data() { data = 0x0; }

void MotorControl::reset_status() { status = 0x0; }

/*
 * Public Functions
 */

void MotorControl::move() {
    // Here we need to sync the clock line and pulse
    // still not 100% how to do that.

    Set_Enable_Pin();
    Set_Direction();
    const int tries = 10000;
    for (int i = 0; i < tries; i++) {
        vTaskDelay(1);
        Set_Step();
        vTaskDelay(1);
        Reset_Step();
    }
}

void MotorControl::setup() {
    // GCONF 0x01
    // IHOLD_IRUN 0x1010
    // CHOPCONF 0x8008
    constexpr uint32_t gconf_data = 0x01;
    constexpr uint32_t ihold_irun_data = 0x1010;
    constexpr uint32_t chopconf = 0x8008;
    auto txBuffer = build_command(
        command_byte(Mode::WRITE, MotorRegisters::GCONF), gconf_data);
    spi_comms.transmit_receive(txBuffer, rxBuffer);
    txBuffer = build_command(
        command_byte(Mode::WRITE, MotorRegisters::IHOLD_IRUN), ihold_irun_data);
    spi_comms.transmit_receive(txBuffer, rxBuffer);
    txBuffer = build_command(
        command_byte(Mode::WRITE, MotorRegisters::CHOPCONF), chopconf);
    spi_comms.transmit_receive(txBuffer, rxBuffer);
    process_buffer(rxBuffer, status, data);
}

void MotorControl::process_buffer(const BufferType& rxBuffer, uint8_t& status,
                                  uint32_t& data) {
    auto iter = rxBuffer.cbegin();
    iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), status);
    // NOLINTNEXTLINE(clang-diagnostic-unused-result)
    bit_utils::bytes_to_int(iter, rxBuffer.cend(), data);
}

void MotorControl::get_status() {
    reset_data();
    reset_status();
    auto txBuffer = build_command(
        command_byte(Mode::READ, MotorRegisters::DRVSTATUS), data);
    spi_comms.transmit_receive(txBuffer, rxBuffer);
    process_buffer(rxBuffer, status, data);
}

void MotorControl::stop() {
    Reset_Step();
    Reset_Enable_Pin();
}
