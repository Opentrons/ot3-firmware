#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <span>

#include "common/core/bit_utils.hpp"
//#include "pipettes/firmware/motor_control.hpp"

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

using namespace bit_utils;

namespace test_motor_control {

const uint8_t WRITE = 0x80;

enum class MotorRegisters : uint8_t {
    GCONF = 0x00,
    DRVSTATUS = 0x6F,
};

class TestMotorControl {
  public:
    std::string state;
    uint8_t status = 0x0;
    uint32_t data = 0x0;

    void setup() { state = "setup"; }
    void stop() { state = "stopped"; }
    void get_status() {
        status = 0x0;
        data = 0x0;
        state = "got_status";
    }
    void set_speed(uint32_t speed) { data = speed; }
    uint32_t get_speed() { return 0x00; }
    void move() { state = "moved"; }

  private:
    void build_command(uint8_t command, uint32_t& command_data,
                       std::array<uint8_t, 5>& txBuffer) {}

    void reset_data() { data = 0x0; }
    void reset_status() { status = 0; }
};

}  // namespace test_motor_control
