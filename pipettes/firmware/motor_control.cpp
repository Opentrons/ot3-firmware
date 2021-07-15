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
                                 std::array<uint8_t, 5>& txBuffer) {
    // need to pass in data parameter and use int_to_bytes here
    auto output = txBuffer.begin();
    output = bit_utils::int_to_bytes(command, output);
    output = bit_utils::int_to_bytes(data, output);
}

void MotorControl::reset_data() {
    data = 0x0;
}

void MotorControl::reset_status() {
    status = 0x0;
}

/*
 * Public Functions
 */

void MotorControl::move() {
    // Here we need to sync the clock line and pulse
    // still not 100% how to do that.
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET);
    Set_Direction();
    while (1) {
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
    auto txBuffer = std::array<uint8_t, 5>{};
    uint32_t gconf_data = 0x01;
    uint32_t ihold_irun_data = 0x1010;
    uint32_t chopconf = 0x8008;
    build_command(WRITE | static_cast<uint8_t>(MotorRegisters::GCONF),
                  gconf_data, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
    build_command(WRITE | static_cast<uint8_t>(MotorRegisters::IHOLD_IRUN),
                  ihold_irun_data, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
    build_command(WRITE | static_cast<uint8_t>(MotorRegisters::CHOPCONF),
                  chopconf, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
}

void MotorControl::get_status() {
    auto txBuffer = std::array<uint8_t, 5>{};
    reset_data();
    reset_status();
    build_command(static_cast<uint8_t>(MotorRegisters::DRVSTATUS), data, txBuffer);
    spi_comms.send_command(txBuffer, data, status);
}

void MotorControl::stop() {
    Reset_Step();
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);


}