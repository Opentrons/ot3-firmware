#pragma once

#include <array>
#include <cstdint>
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

static constexpr auto BUFFER_SIZE = 5;
using BufferType = std::array<uint8_t, BUFFER_SIZE>;

class TestMotorControl {
  public:
    std::string state = "init";
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
    [[nodiscard]] auto get_current_status() const -> uint8_t { return status; }
    [[nodiscard]] auto get_current_data() const -> uint32_t { return data; }
    static void process_buffer(const BufferType& rxBuffer, uint8_t& status,
                               uint32_t& data) {
        auto iter = rxBuffer.cbegin();
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), status);
        // NOLINTNEXTLINE(clang-diagnostic-unused-result)
        iter = bit_utils::bytes_to_int(iter, rxBuffer.cend(), data);
    }

  private:
    uint8_t status = 0x0;
    uint32_t data = 0x0;
    void build_command(uint8_t command, uint32_t& command_data,
                       std::array<uint8_t, 5>& txBuffer) {}

    void reset_data() { data = 0x0; }
    void reset_status() { status = 0; }
};

}  // namespace test_motor_control
