#pragma once

#include <array>
#include <cstdio>

#include "pipettes/core/bit_utils.hpp"


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

const uint8_t WRITE = 0x80;

enum class MotorRegisters : uint8_t {
    GCONF = 0x00,
    DRVSTATUS = 0x6F,
};


class TestMotorControl {
  public:
    uint8_t status = 0x0;
    uint32_t data = 0x0;
    TestMotorControl();
    void setup() {
        auto txBuffer = std::array<uint8_t, 5>{};
        uint32_t gconf_data = 0x01;
        build_command(WRITE | static_cast<uint8_t>(MotorRegisters::GCONF),
                      gconf_data, txBuffer);

    }
    void get_status() {
        auto txBuffer = std::array<uint8_t, 5>{};
        reset_data();
        reset_status();
        build_command(static_cast<uint8_t>(MotorRegisters::DRVSTATUS), data, txBuffer);
    }

  private:
    void build_command(uint8_t command, uint32_t& command_data,
                       std::array<uint8_t, 5>& txBuffer) {
        auto output = txBuffer.begin();
        output = bit_utils::int_to_bytes(command, output);
        output = bit_utils::int_to_bytes(command_data, output);
        status = *output;
        std::span sp{txBuffer};
        bit_utils::bytes_to_int<uint32_t, uint8_t>(sp.subspan(1, 4), data);


    }
    void reset_data();
    void reset_status();
};
