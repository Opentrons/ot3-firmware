#pragma once

#include <array>
#include <cstdint>
#include <iostream>
#include <span>

#include "common/core/bit_utils.hpp"

/*
 * Test mock for MotorControl
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
