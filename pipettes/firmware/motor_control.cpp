#include "pipettes/firmware/motor_control.hpp"

#include "FreeRTOS.h"
#include "firmware/common/motor.h"
#include "pipettes/core/bit_utils.hpp"
#include "stm32g4xx_hal.h"
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
void MotorControl::build_command(uint8_t command, uint32_t& data,
                                 unsigned char* output) {
    // need to pass in data parameter and use int_to_bytes here
    output = bit_utils::int_to_bytes(command, output);
    output = bit_utils::int_to_bytes(data, output);
}

uint32_t MotorControl::reset_data() {
    uint32_t clear = 0x0;
    return data & clear;
}

uint32_t MotorControl::reset_status() {
    uint32_t clear = 0x0;
    return status & clear;
}

/*
 * Public Functions
 */
// void MotorControl::set_speed(uint32_t s) {
//     // This should just save a global variable called
//     // speed. We should decide on our pulse per rotation
//     // based on this number.
//     //    auto txBuffer = std::array<uint8_t, 5>{};
//     //    build_command(MotorRegisters.WRITE | MotorRegisters, txBuffer);
//     //    spi_comms.send_command(txBuffer, data, status);
// }

// auto MotorControl::get_speed() {
//     //    auto txBuffer = std::array<uint8_t, 5>{};
//     //    build_command(MotorRegisters.WRITE | MotorRegisters, data,
//     txBuffer);
//     //    spi_comms.send_command(txBuffer, data, status);
// }

void MotorControl::move() {
    // Here we need to sync the clock line and pulse
    // still not 100% how to do that.
    Set_Direction();
    while (1) {
        vTaskDelay(1);
        Set_Step();
        vTaskDelay(1);
        stop();
    }
}

void MotorControl::setup() {
    // GCONF 0x01
    // IHOLD_IRUN 0x1010
    // CHOPCONF 0x8008
    auto txBuffer = std::array<uint8_t, 5>{};
    auto txiter = txBuffer.begin();
    uint32_t gconf_data = 0x01;
    uint32_t ihold_irun_data = 0x1010;
    uint32_t chopconf = 0x8008;
    build_command(WRITE | static_cast<uint8_t>(MotorRegisters::GCONF),
                  gconf_data, txiter);
    spi_comms.send_command(txBuffer, data, status);
    build_command(WRITE | static_cast<uint8_t>(MotorRegisters::IHOLD_IRUN),
                  ihold_irun_data, txiter);
    spi_comms.send_command(txBuffer, data, status);
    build_command(WRITE | static_cast<uint8_t>(MotorRegisters::CHOPCONF),
                  chopconf, txiter);
    spi_comms.send_command(txBuffer, data, status);
}

void MotorControl::get_status() {
    auto txBuffer = std::array<uint8_t, 5>{};
    auto txiter = txBuffer.begin();
    build_command(static_cast<uint8_t>(MotorRegisters::GSTAT), data, txiter);
    spi_comms.send_command(txBuffer, data, status);
}

void MotorControl::stop() { Reset_Step(); }